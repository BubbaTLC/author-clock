---
name: "Find Missing Quotes"
description: "Search for literary quotes to fill missing minutes in data.json. Asks for search parameters then runs find_missing_quotes.py. Use when you want to find new candidates, run a batch search, or explore what's in the cache."
tools: [read, edit, execute, ask_user]
model: "claude-sonnet-4"
argument-hint: "Optionally describe what you want: e.g. 'run a live search for 50 times', 'use only the cache', 'search everything missing'"
---

Load the `find-missing-quotes` skill before proceeding — it contains the full reference for how the search works.

## Your Job

You help the user find literary quotes for the book-clock's missing minute slots by running `find_missing_quotes.py` with appropriate parameters.

**Always do these steps in order:**

### Step 1 — Show current status

Before asking anything, run this to show the user what they're working with:

```bash
cd /Users/bubbachabot/3d_printing_projects/book-clock && python3 - <<'EOF'
import json
from pathlib import Path

data = json.loads(Path("data.json").read_text())
present = {e["time"] for e in data}
missing = [f"{h:02d}:{m:02d}" for h in range(24) for m in range(60) if f"{h:02d}:{m:02d}" not in present]

candidates_path = Path("tools/candidates.json")
already_found = set()
if candidates_path.exists():
    candidates = json.loads(candidates_path.read_text())
    already_found = {e["time"] for e in candidates}

cache_count = len(list(Path("tools/.search_cache").glob("*.json"))) if Path("tools/.search_cache").exists() else 0

print(f"Missing times      : {len(missing)}")
print(f"In candidates.json : {len(already_found)}")
print(f"Still to find      : {len([t for t in missing if t not in already_found])}")
print(f"Cache entries      : {cache_count}")
EOF
```

### Step 2 — Ask the user for parameters

Ask the following questions **in a single message** (don't ask one at a time):

1. **How many times to process?**
   - Suggest: `25` for a quick test, `100` for a good batch, `all` for everything remaining
   - Mention the current "still to find" count from Step 1

2. **Live search or cache-only?**
   - **Cache-only** (`--cache-only`): Instant, no network — only uses already-cached API responses. Good for re-processing with improved extraction logic.
   - **Live search**: Hits the Google Books API. Each query fires twice (partial-preview books first, then unfiltered fallback). Requires network; ~1–2 sec per time slot.
   - Mention the current cache entry count from Step 1.

3. **Verbose output?** Yes/no — verbose shows every URL fetched/skipped per time slot.

4. **Max candidates per time?** Default is 3. Increase to 5 if you want more options to review.

Make clear that all defaults are sensible and the user can just say "go with defaults" or "just run it".

### Step 3 — Confirm and run

Echo the command you're about to run, then execute it:

```bash
cd /Users/bubbachabot/3d_printing_projects/book-clock && python3 tools/find_missing_quotes.py \
  [--limit N] \
  [--cache-only] \
  [--max-per-time N] \
  [--verbose]
```

Stream output as it runs. The script writes incrementally so progress is never lost if interrupted.

### Step 4 — Report results

After the run, show:
- Times processed / times with hits / new candidates found
- Total entries now in `candidates.json`
- A sample of 3 found quotes (pick ones with good surrounding context — time in the middle, not at the start)
- Any times that returned `✗ none found` — note these as needing a live search if the user ran cache-only

### Step 5 — Offer next steps

Suggest:
- **Review**: `python3 tools/review_candidates.py` to browse and approve/reject entries
- **More searching**: Run live if cache-only was used, or increase `--queries-per-time` for stubborn times
- **Merge**: Once reviewed, merge approved entries into `data.json`

---

## Key Facts

- Project root: `/Users/bubbachabot/3d_printing_projects/book-clock`
- Script: `tools/find_missing_quotes.py`
- Output: `tools/candidates.json` (never modifies `data.json`)
- Cache: `tools/.search_cache/` — 1 JSON file per URL, no TTL
- `--cache-only` skips any URL not already cached (shows `[skip]` in verbose)
- Live runs fire `filter=partial` first (books with preview pages → richer snippets), then unfiltered fallback
- Scoring bonuses: new title +2.0, ALL_PAGES +1.5, PARTIAL preview +1.0, ~200 chars +up to 1.0, clean ending +0.5
- Quotes >500 chars get `"review": true` — these need manual shortening before merging
- Re-runs safely skip times already in `candidates.json`
