"""
time_strings.py — Convert an HH:MM time to a list of search strings.

Generates both numeric and word-form representations so we can find
literary quotes for any minute of the day.

Usage:
    from time_strings import time_search_strings
    for s in time_search_strings("01:03"):
        print(s)
"""

from __future__ import annotations

# 12-hour word forms; index == 12-hour value (0→"twelve" for midnight/noon)
_HOUR_WORDS: list[str] = [
    "twelve",  # 0 (12 AM / 12 PM)
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten",
    "eleven",
    "twelve",
]

_ONES: list[str] = [
    "",
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten",
    "eleven",
    "twelve",
    "thirteen",
    "fourteen",
    "fifteen",
    "sixteen",
    "seventeen",
    "eighteen",
    "nineteen",
]

_TENS: list[str] = ["", "", "twenty", "thirty", "forty", "fifty"]


def _minute_word(m: int) -> str:
    """Return the English word(s) for a minute value 1–59."""
    if m <= 0 or m > 59:
        return ""
    if m < 20:
        return _ONES[m]
    t, o = divmod(m, 10)
    return _TENS[t] + ("-" + _ONES[o] if o else "")


def _to_12h(h: int) -> tuple[int, str]:
    """Convert 24-hour to (12h_value, 'am'|'pm')."""
    if h == 0:
        return 12, "am"
    if h < 12:
        return h, "am"
    if h == 12:
        return 12, "pm"
    return h - 12, "pm"


def time_search_strings(time_str: str) -> list[str]:
    """
    Return a deduplicated, sorted list of search strings for *time_str*.

    *time_str* must be in "HH:MM" 24-hour format.

    Includes numeric and word forms, covering the idioms most commonly
    found in literary fiction (e.g. "three minutes past one", "one-oh-three",
    "quarter to two", "half past midnight", "0103h", "1:03 am").
    """
    h, m = map(int, time_str.split(":"))
    h12, ampm = _to_12h(h)
    h_word = _HOUR_WORDS[h12]
    m_word = _minute_word(m)

    results: set[str] = set()

    # ── Numeric forms ──────────────────────────────────────────────────────────
    ampm_upper = ampm.upper()
    ampm_dotted = "a.m." if ampm == "am" else "p.m."
    ampm_dotted_upper = ampm_dotted.upper()

    # 12-hour with am/pm — plain, dotted, and uppercase variants
    for sep in (":", "."):
        base = f"{h12}{sep}{m:02d}"
        for suffix in (
            f" {ampm}",
            ampm,
            f" {ampm_upper}",
            ampm_upper,
            f" {ampm_dotted}",
            ampm_dotted,
            f" {ampm_dotted_upper}",
            ampm_dotted_upper,
        ):
            results.add(f"{base}{suffix}")
    # Without am/pm designator
    results.add(f"{h12}:{m:02d}")
    results.add(f"{h12:02d}:{m:02d}")
    # 24-hour / military
    results.add(f"{h:02d}:{m:02d}")
    results.add(f"{h:02d}{m:02d}h")
    results.add(f"{h:02d}{m:02d} hours")
    results.add(f"{h:02d}{m:02d}")

    # ── Word forms ─────────────────────────────────────────────────────────────
    if m == 0:
        results.add(f"{h_word} o'clock")
        results.add(f"{h_word} oclock")
        results.add(f"{h_word} on the dot")

    elif m == 15:
        results.add(f"quarter past {h_word}")
        results.add(f"quarter-past {h_word}")
        results.add(f"fifteen past {h_word}")
        results.add(f"fifteen minutes past {h_word}")
        results.add(f"quarter after {h_word}")

    elif m == 30:
        results.add(f"half past {h_word}")
        results.add(f"half-past {h_word}")
        results.add(f"thirty minutes past {h_word}")
        results.add(f"thirty past {h_word}")

    elif m == 45:
        next_h12 = (h12 % 12) + 1
        next_h_word = _HOUR_WORDS[next_h12]
        results.add(f"quarter to {next_h_word}")
        results.add(f"quarter-to {next_h_word}")
        results.add(f"fifteen to {next_h_word}")
        results.add(f"fifteen minutes to {next_h_word}")
        results.add(f"three-quarters past {h_word}")

    else:
        if m < 30:
            # "X minutes past Y" style
            results.add(f"{m_word} minutes past {h_word}")
            results.add(f"{m_word} past {h_word}")
            results.add(f"{m_word} minutes after {h_word}")
            results.add(f"{m_word} after {h_word}")
        else:
            # "X minutes to Y" style
            mins_to = 60 - m
            mins_to_word = _minute_word(mins_to)
            next_h12 = (h12 % 12) + 1
            next_h_word = _HOUR_WORDS[next_h12]
            results.add(f"{mins_to_word} minutes to {next_h_word}")
            results.add(f"{mins_to_word} to {next_h_word}")
            results.add(f"{mins_to_word} minutes before {next_h_word}")
            results.add(f"{mins_to_word} before {next_h_word}")

        # "hour oh minute" for single-digit minutes (e.g. "one oh three")
        if 1 <= m <= 9:
            results.add(f"{h_word} oh {_ONES[m]}")
            results.add(f"{h_word}-oh-{_ONES[m]}")

        # Simple "hour minute" (e.g. "one three", "one twenty-three")
        results.add(f"{h_word} {m_word}")

    return sorted(results)


# ── Priority query list ────────────────────────────────────────────────────────
# For rate-limited searches, use only the most "literary" queries first.


def priority_queries(time_str: str, limit: int = 5) -> list[str]:
    """
    Return the top *limit* most likely search strings for literary quotes.

    These are ordered by how often such phrasing appears in published fiction:
      1. Context-prefixed word idioms ("it was X past Y") → time in mid-sentence
      2. Bare word idioms (half past, quarter to, X minutes past)
      3. Simple "hour oh minute" / "hour minute"
      4. Numeric 12h with am/pm
      5. Numeric 24h/military

    Context prefixes ("it was", "at") encourage Google Books to return snippets
    where the time phrase appears mid-sentence rather than at the very start.
    """
    h, m = map(int, time_str.split(":"))
    h12, ampm = _to_12h(h)
    h_word = _HOUR_WORDS[h12]
    m_word = _minute_word(m)

    ordered: list[str] = []

    if m == 0:
        bare = f"{h_word} o'clock"
        ordered += [
            f"it was {bare}",
            f"at {bare}",
            bare,
            f"by {bare}",
            f"{h12}:{m:02d} {ampm}",
        ]
    elif m == 15:
        bare = f"quarter past {h_word}"
        ordered += [
            f"it was {bare}",
            f"at {bare}",
            bare,
            f"quarter after {h_word}",
            f"{h12}:{m:02d} {ampm}",
        ]
    elif m == 30:
        bare = f"half past {h_word}"
        ordered += [
            f"it was {bare}",
            f"at {bare}",
            bare,
            f"half-past {h_word}",
            f"{h12}:{m:02d} {ampm}",
        ]
    elif m == 45:
        next_h12 = (h12 % 12) + 1
        next_h_word = _HOUR_WORDS[next_h12]
        bare = f"quarter to {next_h_word}"
        ordered += [
            f"it was {bare}",
            f"at {bare}",
            bare,
            f"quarter-to {next_h_word}",
            f"{h12}:{m:02d} {ampm}",
        ]
    elif m < 30:
        bare = f"{m_word} minutes past {h_word}"
        ordered += [
            f"it was {bare}",
            f"at {bare}",
            bare,
            f"{m_word} past {h_word}",
        ]
        if 1 <= m <= 9:
            ordered.append(f"{h_word} oh {_ONES[m]}")
        ordered += [
            f"{h12}:{m:02d} {ampm}",
            f"{h12}.{m:02d}{ampm}",
        ]
    else:
        mins_to = 60 - m
        mins_to_word = _minute_word(mins_to)
        next_h12 = (h12 % 12) + 1
        next_h_word = _HOUR_WORDS[next_h12]
        bare = f"{mins_to_word} minutes to {next_h_word}"
        ordered += [
            f"it was {bare}",
            f"at {bare}",
            bare,
            f"{mins_to_word} to {next_h_word}",
            f"{h12}:{m:02d} {ampm}",
            f"{h12}.{m:02d}{ampm}",
        ]

    # De-duplicate while preserving order
    seen: set[str] = set()
    result: list[str] = []
    for q in ordered:
        if q not in seen:
            seen.add(q)
            result.append(q)
    return result[:limit]


if __name__ == "__main__":
    import sys

    t = sys.argv[1] if len(sys.argv) > 1 else "01:03"
    print(f"All strings for {t}:")
    for s in time_search_strings(t):
        print(f"  {s!r}")
    print("\nPriority queries (top 5):")
    for s in priority_queries(t):
        print(f"  {s!r}")
