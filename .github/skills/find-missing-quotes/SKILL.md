---
name: find-missing-quotes
description: "Find literary book quotes for minutes not yet covered in data.json. Use when: identifying missing times, running the quote search tool, reviewing candidates.json, tuning search quality, adding Google CSE support, or merging candidates into data.json."
argument-hint: "Describe the task: e.g. 'run a batch of 50 missing times', 'check how many times are still missing', 'add google CSE keys', 'merge candidates into data.json'"
---

# Finding Missing Quotes for the Book-Clock

## Overview

`data.json` indexes literary quotes by the minute they reference. Of the 1440 possible
minutes (00:00–23:59), **528 are currently missing**. Two tools in `tools/` handle finding
and vetting new quotes:

| File | Purpose |
|------|---------|
| `tools/time_strings.py` | Converts `HH:MM` → search strings (numeric + word forms) |
| `tools/find_missing_quotes.py` | Orchestrates searches, filters results, writes candidates |
| `tools/candidates.json` | Output — human-reviewed before merging into `data.json` |
| `tools/.search_cache/` | Disk cache of all API responses (gitignored) |

`data.json` is **never modified** by these tools. All output goes to `candidates.json`.

---

## Quick Start

```bash
cd /Users/bubbachabot/3d_printing_projects/book-clock

# Check how many times are missing
python3 -c "
import json
data = json.load(open('data.json'))
present = {e['time'] for e in data}
missing = [f'{h:02d}:{m:02d}' for h in range(24) for m in range(60) if f'{h:02d}:{m:02d}' not in present]
print(f'{len(missing)} times missing')
"

# Process a batch of 50 missing times
python3 tools/find_missing_quotes.py --limit 50

# Process ALL missing times (long — use batches on free API tier)
python3 tools/find_missing_quotes.py
```

Re-runs automatically skip times already present in `candidates.json`, so it's safe to run repeatedly.

---

## How the Search Works

### 1. Time String Generation (`time_strings.py`)

For each missing `HH:MM`, generates two families of search strings:

**Numeric forms** (all combinations):
- `1:03`, `01:03`, `1.03`, `01.03`
- `1:03 am`, `1:03am`, `1:03 AM`, `1.03am`, `1.03 a.m.`, `1.03 A.M.` …
- `0103h`, `0103 hours`, `01:03`

**Word forms** (most "literary"):
- `three minutes past one`, `three past one`
- `one oh three`, `one-oh-three` (for single-digit minutes)
- Context-prefixed: `it was three minutes past one` (places time mid-sentence)
- Special: `half past six`, `quarter to two`, `quarter past three`, `five o'clock`

The `priority_queries(time, limit=5)` function returns the best 5 queries ordered
by how commonly each phrasing appears in published fiction.

### 2. Google Books API (primary, free)

Query: `"<phrase>" subject:fiction` — biases results toward novels and literary works.

Returns `searchInfo.textSnippet` (~160–500 chars) centered on the matched phrase.

**Two-pass fetch per query:**
1. First fires with `filter=free-ebooks` — free Google eBooks with best text access.
2. If still short of candidates, fires with `filter=full` (complete text viewable).
3. Finally, fires with `filter=partial` (books with preview pages) which return 
   richer snippets with full sentence context *before* the time phrase,
   rather than starting right at it (`... <b>time</b> ...`).

All searches use filters to ensure surrounding text context. No unfiltered searches 
are performed.

Books with preview access (`PARTIAL` / `ALL_PAGES` viewability) score higher in
ranking — see Candidate Ranking below.

### 3. Google Custom Search (optional, requires API keys)

Set in `.env`:
```
GOOGLE_CSE_KEY=your_api_key_here
GOOGLE_CSE_CX=your_search_engine_id_here
```

Searches the web (can be scoped to literary sites like `goodreads.com`).
Free tier: 100 queries/day.

### 4. Extraction & Filtering

For each API snippet:
1. Strip HTML tags and entities (`<b>`, `&amp;`, etc.)
2. Normalize `a.m.`/`A.M.` → `am` for reliable matching
3. Walk LEFT from the time phrase to the nearest sentence boundary (up to 300 chars before)
4. Walk RIGHT from the time phrase to the nearest sentence boundary (up to 450 chars after)
5. Result: quote with time at **start, middle, or end** depending on context
6. Reject if the quote doesn't contain a recognisable time string
7. Reject if `_is_literary()` flags it (government reports, reference tables, tabular data)

### 5. Candidate Ranking

Per time slot, keeps up to **3 candidates** scored as follows (higher wins):

| Signal | Score |
|--------|-------|
| Title not already in `data.json` | +2.0 |
| Book has `ALL_PAGES` preview access | +1.5 |
| Book has `PARTIAL` preview access | +1.0 |
| Quote length near 200 chars | up to +1.0 (linear penalty away from 200) |
| Quote ends at clean sentence boundary | +0.5 |

Preview access correlates directly with richer snippets: Google Books returns
sentence context *before* the time phrase only when it can show pages.

---

## Reviewing `candidates.json`

```json
[
  {
    "time": "01:07",
    "timeString": "seven minutes past one",
    "quote": "It was seven minutes past one and she wasn't alarmed by the echo of her days...",
    "title": "Bring Me Sunshine",
    "author": "Linda Grant"
  },
  {
    "time": "01:07",
    "timeString": "seven minutes past one",
    "quote": "...",
    "title": "The Mind and Body Shop",
    "author": "Frank Parkin",
    "review": true
  }
]
```

- Remove entries with bad quotes (reference books, garbled OCR, non-fiction)
- Shorten any `"review": true` entries to a clean sentence
- Update `"timeString"` if the found phrase is better than the search query used

## Merging into `data.json`

After review, append accepted entries (strip the `"review"` field first):

```python
import json

with open('tools/candidates.json') as f:
    candidates = json.load(f)

with open('data.json') as f:
    data = json.load(f)

# Strip review flag and add to data
clean = [{k: v for k, v in e.items() if k != 'review'} for e in candidates]
data.extend(clean)
data.sort(key=lambda e: e['time'])

with open('data.json', 'w') as f:
    json.dump(data, f, indent=2, ensure_ascii=False)
```

Then regenerate the binary:
```bash
python3 tools/gen_quotes_bin.py
idf.py partition-table-flash
```

---

## CLI Reference

```
python3 tools/find_missing_quotes.py [options]

  --data PATH           Path to data.json             (default: data.json)
  --output PATH         Path to candidates.json        (default: tools/candidates.json)
  --limit N             Process at most N missing times this run
  --queries-per-time N  Max search queries per time    (default: 5)
  --max-per-time N      Max candidates kept per time   (default: 3)
  --cache-dir PATH      API response cache directory   (default: tools/.search_cache)
  --no-books            Skip Google Books API
  --no-cse              Skip Google Custom Search API
  --delay SECS          Sleep between HTTP requests    (default: 1.0)
  --cache-only          Use only cached responses; skip any URL not yet cached
  --verbose / -v        Print per-query detail
```

---

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `✗ none found` for most times | No fiction-tagged books matched | Try `--no-books` if Books API is rate-limited; check cache |
| `HTTP Error 503` | Occasional Google API hiccup | Script handles gracefully; cached responses fill in |
| All quotes have time at start | Google snippet window opened right at the time; `NO_PAGES` viewability | These are ranked lower automatically; live runs prefer `filter=partial` books first |
| Non-fiction slipping through | `_is_literary()` filter miss | Edit the `_SKIP_AUTHOR_TOKENS` set in `find_missing_quotes.py` |
| `candidates.json` grows stale | Re-ran after adding candidates to `data.json` | Delete `candidates.json`; script rebuilds from scratch |
| Rate limit exhausted | Google CSE 100/day free limit | Use `--limit 50` and run again tomorrow; Books API is unlimited |

---

## File Locations

```
tools/
  time_strings.py           # Time → search string conversion
  find_missing_quotes.py    # Main search orchestrator
  candidates.json           # Output (gitignored; human-reviewed)
  .search_cache/            # API response cache (gitignored)
```
