"""
find_missing_quotes.py — Search for literary quotes for missing book-clock times.

Identifies every minute not covered by data.json, then queries the Google Books
API (free, no key needed) and optionally Google Custom Search API (requires
GOOGLE_CSE_KEY + GOOGLE_CSE_CX in .env) to find candidate quotes.

Output: tools/candidates.json  — same format as data.json, never touches data.json.
        Each entry may also carry:
          "source":  URL or API name the snippet came from
          "review":  true  if the quote is >500 chars and may need shortening

Usage:
    cd <project-root>
    python3 tools/find_missing_quotes.py [options]

Options:
    --data PATH         Path to data.json  (default: data.json)
    --output PATH       Path to candidates.json  (default: tools/candidates.json)
    --limit N           Process at most N missing times per run  (default: all)
    --queries-per-time N  Max search queries per missing time  (default: 5)
    --max-per-time N    Max candidates kept per time  (default: 3)
    --cache-dir PATH    Directory for cached API responses  (default: tools/.search_cache)
    --no-books          Skip Google Books API
    --no-cse            Skip Google Custom Search API (even if keys are present)
    --delay SECS        Seconds to sleep between HTTP requests  (default: 1.0)
    --verbose           Print progress details
"""

from __future__ import annotations

import argparse
import html
import json
import re
import sys
import time
import urllib.parse
import urllib.request
from pathlib import Path
from typing import Any


# ── Helpers ───────────────────────────────────────────────────────────────────


def _load_env(env_path: Path) -> dict[str, str]:
    """Parse a simple KEY=VALUE .env file (no shell expansion)."""
    env: dict[str, str] = {}
    if not env_path.exists():
        return env
    for line in env_path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, _, val = line.partition("=")
        env[key.strip()] = val.strip().strip('"').strip("'")
    return env


def _get(url: str, cache_dir: Path, verbose: bool) -> dict[str, Any] | None:
    """Fetch *url* as JSON, using *cache_dir* as a disk cache."""
    cache_key = re.sub(r"[^a-zA-Z0-9_\-]", "_", url)[:200]
    cache_file = cache_dir / f"{cache_key}.json"

    if cache_file.exists():
        if verbose:
            print(f"    [cache] {url[:80]}")
        return json.loads(cache_file.read_text())

    if verbose:
        print(f"    [fetch] {url[:80]}")
    try:
        req = urllib.request.Request(
            url,
            headers={"User-Agent": "book-clock-quote-finder/1.0"},
        )
        with urllib.request.urlopen(req, timeout=15) as resp:
            raw = resp.read()
        data = json.loads(raw)
        cache_file.write_text(json.dumps(data))
        return data
    except Exception as exc:
        print(f"    [error] {exc}", file=sys.stderr)
        return None


# ── Text extraction ────────────────────────────────────────────────────────────

_HTML_TAG = re.compile(r"<[^>]+>")
_WHITESPACE = re.compile(r"\s+")
_SENTENCE_END = re.compile(r"[.?!…]")

_MIN_LEN = 50
_MAX_LEN = 688  # hard cap matching longest entry in data.json
_REVIEW_LEN = 500

# Normalize dotted am/pm so "1:02 a.m." matches query "1:02 am"
_DOTTED_AMPM = re.compile(r"\b([aApP])\.([mM])\.", re.IGNORECASE)


def _normalize_ampm(text: str) -> str:
    """Replace 'a.m.'/'p.m.' with 'am'/'pm' for consistent matching."""
    return _DOTTED_AMPM.sub(lambda m: m.group(1) + m.group(2), text).lower()


# Patterns that suggest non-literary content (reports, tables, reference books)
_SKIP_AUTHOR_TOKENS = frozenset(
    [
        "parliament",
        "congress",
        "committee",
        "inspectors",
        "department",
        "government",
        "ministry",
        "senate",
        "house of",
        "llewellyn",
        "bureau",
        "office of",
        "proceedings",
    ]
)
_TABULAR_RE = re.compile(r"(\d{1,2}:\d{2}\s*[ap]m?\s*){3,}", re.IGNORECASE)


def _is_literary(quote: str, title: str, author: str, categories: list[str]) -> bool:
    """Return True if this looks like a literary (fiction/narrative) source."""
    author_lower = author.lower()
    if any(tok in author_lower for tok in _SKIP_AUTHOR_TOKENS):
        return False

    # Reject tabular content (three or more times in a row)
    if _TABULAR_RE.search(quote):
        return False

    # Reject if majority of words are numbers/short tokens (table rows)
    words = quote.split()
    if len(words) > 5:
        num_tokens = sum(1 for w in words if re.fullmatch(r"[\d:./,\-]+", w))
        if num_tokens / len(words) > 0.35:
            return False

    # Accept if categories mention fiction or literature
    cats_lower = " ".join(categories).lower()
    if any(
        k in cats_lower for k in ("fiction", "literature", "novel", "poetry", "drama")
    ):
        return True

    # Neutral — accept unless filtered above
    return True


def _clean(text: str) -> str:
    """Strip HTML tags and entities, collapse whitespace."""
    text = _HTML_TAG.sub("", text)
    text = html.unescape(text)
    return _WHITESPACE.sub(" ", text).strip()


def _extract_quote(snippet: str, time_phrase: str) -> str | None:
    """
    From an API snippet, extract the best sentence-bounded quote containing
    *time_phrase*.  Returns None if no usable text is found.

    The extracted quote may have the time phrase at the start, middle, or end —
    whatever the surrounding sentence context provides.
    """
    text = _clean(snippet)
    if not text:
        return None

    # Normalize both text and phrase for position matching (handles a.m. vs am)
    norm_text = _normalize_ampm(text)
    norm_phrase = _normalize_ampm(time_phrase)
    idx = norm_text.find(norm_phrase)

    if idx == -1:
        # Fallback: use full snippet if it's a reasonable length
        return text if _MIN_LEN <= len(text) <= _MAX_LEN else None

    # ── Walk LEFT collecting up to two sentence boundaries ────────────────────
    # Two boundaries let us include the previous sentence when the time phrase
    # sits near the start of its own sentence (time mid-quote rather than first
    # word).
    left_bounds: list[int] = []
    for i in range(idx - 1, max(idx - 400, -1), -1):
        if text[i] in ".?!…\n":
            pos = i + 1
            while pos < idx and text[pos] == " ":
                pos += 1
            left_bounds.append(pos)
            if len(left_bounds) == 2:
                break

    if not left_bounds:
        stripped_start = text.lstrip(". …\t\n")
        left = len(text) - len(stripped_start) if text != stripped_start else 0
    else:
        left = left_bounds[0]
        # If the time phrase starts within 15 chars of its sentence boundary
        # (i.e., time opens the sentence), include the previous sentence so the
        # time lands in the middle of the extracted quote.
        if idx - left < 15 and len(left_bounds) > 1:
            left = left_bounds[1]

    # ── Walk RIGHT to the nearest sentence boundary ────────────────────────────
    phrase_end = idx + len(norm_phrase)
    right = len(text)
    for i in range(phrase_end, min(phrase_end + 450, len(text))):
        if text[i] in ".?!…":
            right = i + 1
            break

    quote = text[left:right].strip()

    # Strip leading ellipsis artifacts that Google Books inserts at snippet
    # boundaries ("...", "…").  These are API markers, not original book text.
    quote = re.sub(r"^[.…]+\s*", "", quote).strip()

    # ── Length guard ────────────────────────────────────────────────────────────
    if len(quote) < _MIN_LEN:
        quote = re.sub(r"^[.…]+\s*", "", text).strip()
    if len(quote) > _MAX_LEN:
        truncated = quote[:_MAX_LEN]
        last = max(truncated.rfind("."), truncated.rfind("?"), truncated.rfind("!"))
        if last > _MIN_LEN:
            quote = truncated[: last + 1].strip()
        else:
            quote = truncated.rstrip()

    return quote if len(quote) >= _MIN_LEN else None


# ── Scoring ───────────────────────────────────────────────────────────────────


def _score(quote: str, title: str, existing_titles: set[str]) -> float:
    """
    Score a candidate (higher is better).
    Prefer: distinct titles, length near 200 chars, clean sentence endings.
    """
    score = 0.0
    if title.lower() not in existing_titles:
        score += 2.0
    length_penalty = abs(len(quote) - 200) / 200.0
    score -= length_penalty
    if quote.rstrip().endswith((".", "?", "!", "…", '"')):
        score += 0.5
    return score


# ── Google Books API ──────────────────────────────────────────────────────────

_BOOKS_BASE = "https://www.googleapis.com/books/v1/volumes"


def _search_books(
    query: str,
    cache_dir: Path,
    verbose: bool,
    api_key: str | None = None,
) -> list[dict[str, Any]]:
    """
    Query Google Books for *query* and return a list of
    {title, author, snippet, categories} dicts.
    Adds `subject:fiction` to bias results toward literary works.
    """
    params: dict[str, str] = {
        "q": f'"{query}" subject:fiction',
        "langRestrict": "en",
        "maxResults": "10",
        "printType": "books",
    }
    if api_key:
        params["key"] = api_key

    url = f"{_BOOKS_BASE}?{urllib.parse.urlencode(params)}"
    data = _get(url, cache_dir, verbose)
    if not data or "items" not in data:
        return []

    results = []
    for item in data["items"]:
        vi = item.get("volumeInfo", {})
        si = item.get("searchInfo", {})
        snippet = si.get("textSnippet", "")
        if not snippet:
            continue
        title = vi.get("title", "")
        authors = vi.get("authors", [])
        author = authors[0] if authors else ""
        categories = vi.get("categories", [])
        if title and author and snippet:
            results.append(
                {
                    "title": title,
                    "author": author,
                    "snippet": snippet,
                    "categories": categories,
                }
            )
    return results


# ── Google Custom Search API ──────────────────────────────────────────────────

_CSE_BASE = "https://www.googleapis.com/customsearch/v1"


def _search_cse(
    query: str,
    api_key: str,
    cx: str,
    cache_dir: Path,
    verbose: bool,
) -> list[dict[str, Any]]:
    """
    Query Google Custom Search for *query* and return snippets.
    Results don't include clean title/author, so they need further validation.
    """
    params = {
        "key": api_key,
        "cx": cx,
        "q": f'"{query}" book quote',
        "num": "10",
    }
    url = f"{_CSE_BASE}?{urllib.parse.urlencode(params)}"
    data = _get(url, cache_dir, verbose)
    if not data or "items" not in data:
        return []

    results = []
    for item in data["items"]:
        snippet = item.get("snippet", "")
        title = item.get("title", "")
        if snippet:
            results.append({"title": title, "author": "", "snippet": snippet})
    return results


# ── Core logic ────────────────────────────────────────────────────────────────


def find_missing_times(data_path: Path) -> list[str]:
    """Return sorted list of HH:MM times not present in *data_path*."""
    with data_path.open(encoding="utf-8") as f:
        data = json.load(f)
    present = {entry["time"] for entry in data}
    missing = []
    for h in range(24):
        for m in range(60):
            t = f"{h:02d}:{m:02d}"
            if t not in present:
                missing.append(t)
    return missing


def process_time(
    time_str: str,
    queries: list[str],
    max_per_time: int,
    cache_dir: Path,
    verbose: bool,
    use_books: bool,
    cse_key: str | None,
    cse_cx: str | None,
    books_key: str | None,
    delay: float,
    existing_titles: set[str],
) -> list[dict[str, Any]]:
    """
    Search all sources for *time_str* and return up to *max_per_time* candidates.
    """
    from time_strings import time_search_strings  # noqa: PLC0415

    all_strings_lower = {s.lower() for s in time_search_strings(time_str)}
    candidates: list[dict[str, Any]] = []
    seen_quotes: set[str] = set()

    def add_result(
        title: str,
        author: str,
        snippet: str,
        query: str,
        categories: list[str] | None = None,
    ) -> None:
        quote = _extract_quote(snippet, query)
        if not quote:
            return
        q_norm = _normalize_ampm(quote)
        # Verify the quote actually contains a recognisable time reference
        all_norm = {_normalize_ampm(s) for s in all_strings_lower}
        if not any(s in q_norm for s in all_norm):
            return
        # Filter out non-literary sources
        if not _is_literary(quote, title, author, categories or []):
            return
        # Deduplicate by first 80 chars
        key = quote[:80].lower()
        if key in seen_quotes:
            return
        seen_quotes.add(key)
        entry: dict[str, Any] = {
            "time": time_str,
            "timeString": query,
            "quote": quote,
            "title": title,
            "author": author,
        }
        if len(quote) > _REVIEW_LEN:
            entry["review"] = True
        candidates.append(entry)

    for query in queries:
        if len(candidates) >= max_per_time * 3:  # gather extras then rank
            break

        if use_books:
            for r in _search_books(query, cache_dir, verbose, books_key):
                add_result(
                    r["title"],
                    r["author"],
                    r["snippet"],
                    query,
                    r.get("categories", []),
                )
            time.sleep(delay)

        if cse_key and cse_cx:
            for r in _search_cse(query, cse_key, cse_cx, cache_dir, verbose):
                add_result(r["title"], r["author"], r["snippet"], query)
            time.sleep(delay)

    if not candidates:
        return []

    # Rank and keep top N
    candidates.sort(
        key=lambda c: _score(c["quote"], c["title"], existing_titles),
        reverse=True,
    )
    # Prefer distinct titles
    seen_titles: set[str] = set()
    top: list[dict[str, Any]] = []
    for c in candidates:
        if len(top) >= max_per_time:
            break
        t_lower = c["title"].lower()
        if t_lower not in seen_titles:
            seen_titles.add(t_lower)
            top.append(c)
    # Fill remaining slots from any title if needed
    if len(top) < max_per_time:
        for c in candidates:
            if len(top) >= max_per_time:
                break
            if c not in top:
                top.append(c)

    return top


# ── CLI ───────────────────────────────────────────────────────────────────────


def main() -> None:
    # Locate project root (parent of tools/)
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent

    parser = argparse.ArgumentParser(description=__doc__.splitlines()[1].strip())
    parser.add_argument("--data", default=str(project_root / "data.json"))
    parser.add_argument("--output", default=str(script_dir / "candidates.json"))
    parser.add_argument(
        "--limit", type=int, default=None, help="Max missing times to process this run"
    )
    parser.add_argument("--queries-per-time", type=int, default=5)
    parser.add_argument("--max-per-time", type=int, default=3)
    parser.add_argument("--cache-dir", default=str(script_dir / ".search_cache"))
    parser.add_argument("--no-books", action="store_true")
    parser.add_argument("--no-cse", action="store_true")
    parser.add_argument("--delay", type=float, default=1.0)
    parser.add_argument("--verbose", "-v", action="store_true")
    args = parser.parse_args()

    data_path = Path(args.data)
    output_path = Path(args.output)
    cache_dir = Path(args.cache_dir)
    cache_dir.mkdir(parents=True, exist_ok=True)

    # Load .env for optional API keys
    env = _load_env(project_root / ".env")
    cse_key = env.get("GOOGLE_CSE_KEY") or None
    cse_cx = env.get("GOOGLE_CSE_CX") or None
    books_key = env.get("GOOGLE_BOOKS_KEY") or None  # optional

    if cse_key and cse_cx:
        print(f"Google CSE: enabled (cx={cse_cx[:8]}...)")
    else:
        print(
            "Google CSE: disabled (set GOOGLE_CSE_KEY + GOOGLE_CSE_CX in .env to enable)"
        )

    if args.no_books:
        use_books = False
    else:
        use_books = True
        print("Google Books API: enabled (free)")

    if args.no_cse:
        cse_key = None

    # Gather existing titles for diversity scoring
    with data_path.open(encoding="utf-8") as f:
        existing_data = json.load(f)
    existing_titles = {e.get("title", "").lower() for e in existing_data}

    missing = find_missing_times(data_path)
    print(f"\nMissing times: {len(missing)}")

    # Load any previously found candidates so we can append / skip already-done
    if output_path.exists():
        with output_path.open(encoding="utf-8") as f:
            prior = json.load(f)
    else:
        prior = []
    already_found = {e["time"] for e in prior}

    to_process = [t for t in missing if t not in already_found]
    if args.limit:
        to_process = to_process[: args.limit]

    print(f"To process this run: {len(to_process)} times")
    if not to_process:
        print("Nothing to do — all missing times already have candidates.")
        return

    # Ensure time_strings module is importable from tools/
    sys.path.insert(0, str(script_dir))
    from time_strings import priority_queries  # noqa: PLC0415

    new_candidates: list[dict[str, Any]] = []
    found_count = 0

    for i, t in enumerate(to_process, 1):
        queries = priority_queries(t, limit=args.queries_per_time)
        if args.verbose:
            print(f"\n[{i}/{len(to_process)}] {t}  queries={queries[:3]}")
        else:
            print(f"[{i}/{len(to_process)}] {t}", end="  ", flush=True)

        results = process_time(
            time_str=t,
            queries=queries,
            max_per_time=args.max_per_time,
            cache_dir=cache_dir,
            verbose=args.verbose,
            use_books=use_books,
            cse_key=cse_key,
            cse_cx=cse_cx,
            books_key=books_key,
            delay=args.delay,
            existing_titles=existing_titles,
        )

        if results:
            found_count += 1
            if not args.verbose:
                print(f"✓ {len(results)} found")
        else:
            if not args.verbose:
                print("✗ none found")

        new_candidates.extend(results)

        # Write incrementally so progress isn't lost on interrupt
        combined = prior + new_candidates
        output_path.parent.mkdir(parents=True, exist_ok=True)
        with output_path.open("w", encoding="utf-8") as f:
            json.dump(combined, f, indent=2, ensure_ascii=False)

    print("\n── Summary ──────────────────────────────────────────────────────")
    print(f"  Times processed : {len(to_process)}")
    print(f"  Times with hits : {found_count}")
    print(f"  New candidates  : {len(new_candidates)}")
    print(f"  Total in output : {len(prior) + len(new_candidates)}")
    print(f"  Output          : {output_path}")


if __name__ == "__main__":
    main()
