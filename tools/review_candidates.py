#!/usr/bin/env python3
"""
Review candidates.json using the Copilot CLI programmatically.

Splits candidates into batches, sends each batch to:
  copilot -p "..." -s --no-ask-user --model claude-haiku-4.5

Rules applied per batch:
  1. Remove non-English entries
  2. For '...' endings — keep if mysterious/cliffhanger, otherwise trim to last complete sentence
  3. Remove OCR artifacts, reference tables, non-fiction junk

Usage:
  python3 tools/review_candidates.py [--limit N] [--batch-size N] [--resume] [--model MODEL]
"""

import argparse
import json
import re
import subprocess
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).parent.parent
CANDIDATES_FILE = REPO_ROOT / "tools" / "candidates.json"
OUTPUT_FILE = REPO_ROOT / "tools" / "candidates_reviewed.json"
PROGRESS_FILE = REPO_ROOT / "tools" / ".review_progress.json"

REVIEW_PROMPT_TEMPLATE = """\
Review these book-clock quote candidates. Each entry has: time, timeString, quote, title, author.

Apply these rules and return ONLY a valid JSON array of the surviving entries. No explanation.

Rules:
1. REMOVE the entry if the quote text is not in English.
2. If the quote ends with "..." — keep as-is only if the truncated ending adds mystery,
   suspense, or a cliffhanger. Otherwise, trim the quote to end at the last complete sentence
   before the "..." (remove the dangling fragment and trailing "...").
3. REMOVE entries with OCR artifacts (embedded page numbers like "370.", chapter headers
   like "CHAPTER FIVE", asterisks used as scene breaks like "***** *****"), reference tables,
   or non-fiction content (schedules, glossaries, catalogs, time-indexed lists).

Return the JSON array only. Do not include markdown fences or any explanation.

Candidates:
{batch_json}"""


def load_progress():
    if PROGRESS_FILE.exists():
        with open(PROGRESS_FILE) as f:
            return json.load(f)
    return {"completed_batches": [], "results": []}


def save_progress(progress):
    with open(PROGRESS_FILE, "w") as f:
        json.dump(progress, f, indent=2, ensure_ascii=False)


def extract_json_array(text: str) -> list | None:
    """Extract a JSON array from copilot output, stripping markdown fences if present."""
    text = text.strip()
    # Strip markdown code fences
    text = re.sub(r"^```(?:json)?\s*", "", text, flags=re.MULTILINE)
    text = re.sub(r"```\s*$", "", text, flags=re.MULTILINE)
    text = text.strip()
    # Find the outermost JSON array
    start = text.find("[")
    end = text.rfind("]")
    if start == -1 or end == -1:
        return None
    try:
        return json.loads(text[start : end + 1])
    except json.JSONDecodeError:
        return None


def validate_entry(entry: dict) -> bool:
    """Check that a reviewed entry has the required fields."""
    return all(k in entry for k in ("time", "quote", "title", "author"))


def call_copilot(prompt: str, model: str, retries: int = 2) -> str | None:
    """Call copilot CLI and return its stdout, or None on failure."""
    for attempt in range(retries + 1):
        result = subprocess.run(
            ["copilot", "-p", prompt, "-s", "--no-ask-user", f"--model={model}"],
            capture_output=True,
            text=True,
        )
        if result.returncode == 0 and result.stdout.strip():
            return result.stdout
        if attempt < retries:
            print(
                f"  ⚠ copilot returned code {result.returncode}, retrying ({attempt+1}/{retries})…"
            )
            time.sleep(3)
    print(f"  ✗ copilot failed after {retries+1} attempts", file=sys.stderr)
    if result.stderr:
        print(f"    stderr: {result.stderr[:200]}", file=sys.stderr)
    return None


def review_batch(batch: list[dict], batch_idx: int, model: str) -> list[dict]:
    """Send one batch to copilot and return the reviewed entries."""
    # ensure_ascii=True encodes smart-quotes/special chars as \uXXXX so the LLM
    # won't convert them to raw " characters, which would break JSON output parsing.
    batch_json = json.dumps(batch, indent=2, ensure_ascii=True)
    prompt = REVIEW_PROMPT_TEMPLATE.format(batch_json=batch_json)

    raw = call_copilot(prompt, model)
    if raw is None:
        print(
            f"  ✗ Batch {batch_idx}: copilot call failed — keeping original batch as fallback"
        )
        return batch

    entries = extract_json_array(raw)
    if entries is None:
        print(
            f"  ✗ Batch {batch_idx}: could not parse JSON output — keeping original batch as fallback"
        )
        print(f"    Raw output (first 300 chars): {raw[:300]}")
        return batch

    valid = [e for e in entries if validate_entry(e)]
    removed = len(batch) - len(valid)
    print(
        f"  ✓ Batch {batch_idx}: {len(batch)} in → {len(valid)} kept ({removed} removed)"
    )
    return valid


def main():
    parser = argparse.ArgumentParser(
        description="Review candidates.json with Copilot CLI"
    )
    parser.add_argument(
        "--limit", type=int, default=None, help="Process at most N candidates"
    )
    parser.add_argument(
        "--batch-size",
        type=int,
        default=20,
        help="Entries per copilot call (default: 20)",
    )
    parser.add_argument(
        "--model",
        default="claude-haiku-4.5",
        help="Model to use (default: claude-haiku-4.5)",
    )
    parser.add_argument(
        "--resume", action="store_true", help="Resume from saved progress"
    )
    parser.add_argument(
        "--no-resume",
        action="store_true",
        help="Ignore any saved progress and start fresh",
    )
    parser.add_argument(
        "--delay",
        type=float,
        default=1.0,
        help="Seconds between batches (default: 1.0)",
    )
    args = parser.parse_args()

    # Load candidates
    with open(CANDIDATES_FILE) as f:
        candidates = json.load(f)

    if args.limit:
        candidates = candidates[: args.limit]

    total = len(candidates)
    print(f"Loaded {total} candidates from {CANDIDATES_FILE.name}")

    # Load or reset progress
    progress = {"completed_batches": [], "results": []}
    if not args.no_resume and PROGRESS_FILE.exists():
        progress = load_progress()
        if progress["completed_batches"]:
            print(
                f"Resuming: {len(progress['completed_batches'])} batches already done, "
                f"{len(progress['results'])} entries collected so far"
            )

    completed = set(progress["completed_batches"])
    results = progress["results"]

    # Split into batches
    batches = [
        candidates[i : i + args.batch_size] for i in range(0, total, args.batch_size)
    ]
    print(f"Split into {len(batches)} batches of up to {args.batch_size}")
    print(f"Model: {args.model}\n")

    for idx, batch in enumerate(batches):
        if idx in completed:
            print(f"  → Batch {idx}: already done, skipping")
            continue

        print(f"Processing batch {idx + 1}/{len(batches)} ({len(batch)} entries)…")
        reviewed = review_batch(batch, idx, args.model)
        results.extend(reviewed)
        completed.add(idx)
        progress = {"completed_batches": list(completed), "results": results}
        save_progress(progress)

        if idx < len(batches) - 1:
            time.sleep(args.delay)

    # Write output
    with open(OUTPUT_FILE, "w") as f:
        json.dump(results, f, indent=2, ensure_ascii=False)

    total_removed = total - len(results)
    print(f"\n{'='*50}")
    print(
        f"Done! {total} candidates → {len(results)} kept ({total_removed} removed, "
        f"{total_removed*100//total if total else 0}% reduction)"
    )
    print(f"Output: {OUTPUT_FILE}")
    print("\nWhen satisfied, replace the original with:")
    print("  cp tools/candidates_reviewed.json tools/candidates.json")

    # Clean up progress file on success
    if PROGRESS_FILE.exists():
        PROGRESS_FILE.unlink()
        print("  (progress file cleaned up)")


if __name__ == "__main__":
    main()
