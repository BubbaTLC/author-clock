#!/usr/bin/env python3
"""
gen_quotes_bin.py — Generate a compact binary quote database for the ESP32 book-clock.

Binary format (quotes.bin):
  Header (7 bytes):
    [0..3]  Magic:   "BQDB"  (4 ASCII bytes)
    [4]     Version: 0x01
    [5..6]  Count:   uint16_t LE  (total number of quote records)

  Time Index (7200 bytes = 1440 entries × 5 bytes):
    For each minute of the day, index = hour*60 + min (0..1439):
      [0..3]  offset: uint32_t LE  (byte offset from file start to first record)
      [4]     count:  uint8_t      (number of records for this time; 0 = none)

  Records (variable length, contiguous):
    For each record:
      [0..1]  quote_len:  uint16_t LE
      [2]     title_len:  uint8_t
      [3]     author_len: uint8_t
      [4..]   quote:      quote_len bytes (UTF-8, no null terminator)
      [..]    title:      title_len bytes
      [..]    author:     author_len bytes

Lookup on ESP32:
  idx = hour * 60 + min
  seek(7 + idx * 5) → read offset (uint32) + count (uint8)
  seek(offset) → read count records → pick esp_random() % count

Usage:
  python3 tools/gen_quotes_bin.py [input.json] [output.bin]

Defaults:
  input:  data.json          (project root)
  output: main/data/quotes.bin
"""

import json
import struct
import sys
import os
from collections import defaultdict

MAGIC = b"BQDB"
VERSION = 0x01

INDEX_ENTRIES = 1440  # one per minute of the day
INDEX_ENTRY_SIZE = 5  # uint32 offset + uint8 count
INDEX_SIZE = INDEX_ENTRIES * INDEX_ENTRY_SIZE
HEADER_SIZE = 7
RECORDS_START = HEADER_SIZE + INDEX_SIZE  # 7 + 7200 = 7207

MAX_QUOTE_LEN = 0xFFFF
MAX_TITLE_LEN = 0xFF
MAX_AUTHOR_LEN = 0xFF


def parse_time(t: str) -> tuple[int, int]:
    """Parse "HH:MM" → (hour, minute)."""
    parts = t.strip().split(":")
    return int(parts[0]), int(parts[1])


def encode_record(quote: str, title: str, author: str) -> bytes:
    """Encode a single variable-length record."""
    q = quote.encode("utf-8")
    t = title.encode("utf-8")
    a = author.encode("utf-8")

    if len(q) > MAX_QUOTE_LEN:
        print(
            f"  WARNING: quote truncated ({len(q)} → {MAX_QUOTE_LEN} bytes): {quote[:60]}..."
        )
        q = q[:MAX_QUOTE_LEN]
    if len(t) > MAX_TITLE_LEN:
        print(f"  WARNING: title truncated ({len(t)} → {MAX_TITLE_LEN} bytes): {title}")
        t = t[:MAX_TITLE_LEN]
    if len(a) > MAX_AUTHOR_LEN:
        print(
            f"  WARNING: author truncated ({len(a)} → {MAX_AUTHOR_LEN} bytes): {author}"
        )
        a = a[:MAX_AUTHOR_LEN]

    header = struct.pack("<HBB", len(q), len(t), len(a))
    return header + q + t + a


def generate(input_path: str, output_path: str) -> None:
    print(f"Reading: {input_path}")
    with open(input_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    print(f"  {len(data)} total entries")

    # Group records by minute index (0..1439)
    by_minute: dict[int, list[tuple[str, str, str]]] = defaultdict(list)
    skipped = 0
    for entry in data:
        try:
            hour, minute = parse_time(entry["time"])
        except (KeyError, ValueError, IndexError):
            skipped += 1
            continue

        if not (0 <= hour <= 23 and 0 <= minute <= 59):
            skipped += 1
            continue

        idx = hour * 60 + minute
        by_minute[idx].append(
            (
                entry.get("quote", ""),
                entry.get("title", ""),
                entry.get("author", ""),
            )
        )

    if skipped:
        print(f"  {skipped} entries skipped (bad time format)")

    unique_times = len(by_minute)
    total_records = sum(len(v) for v in by_minute.values())
    print(f"  {unique_times} unique times, {total_records} usable records")

    # Build encoded record bytes grouped by minute
    encoded: dict[int, list[bytes]] = {}
    for idx, records in by_minute.items():
        encoded[idx] = [encode_record(q, t, a) for (q, t, a) in records]

    # Build time index: compute each slot's offset into the file
    offset = RECORDS_START  # first record starts right after header + index
    index_entries: list[tuple[int, int]] = []  # (offset, count) per minute

    for idx in range(INDEX_ENTRIES):
        recs = encoded.get(idx, [])
        count = len(recs)
        if count > 255:
            print(f"  WARNING: minute {idx} has {count} records; clamping to 255")
            recs = recs[:255]
            encoded[idx] = recs
            count = 255
        index_entries.append((offset, count))
        offset += sum(len(r) for r in recs)

    total_file_size = offset  # offset is now past the last record

    # Write the binary file
    os.makedirs(os.path.dirname(output_path) or ".", exist_ok=True)
    with open(output_path, "wb") as out:
        # Header
        out.write(MAGIC)
        out.write(struct.pack("<BH", VERSION, total_records))

        # Time index
        for off, cnt in index_entries:
            out.write(struct.pack("<IB", off, cnt))

        # Records, in minute order
        for idx in range(INDEX_ENTRIES):
            for rec in encoded.get(idx, []):
                out.write(rec)

    print(f"\nWrote: {output_path}")
    print(f"  Header:     {HEADER_SIZE} bytes")
    print(
        f"  Time index: {INDEX_SIZE} bytes ({INDEX_ENTRIES} slots × {INDEX_ENTRY_SIZE} bytes)"
    )
    print(f"  Records:    {total_file_size - RECORDS_START} bytes")
    print(f"  Total:      {total_file_size} bytes ({total_file_size / 1024:.1f} KB)")


def main() -> None:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    input_path = (
        sys.argv[1] if len(sys.argv) > 1 else os.path.join(project_root, "data.json")
    )
    output_path = (
        sys.argv[2]
        if len(sys.argv) > 2
        else os.path.join(project_root, "main", "data", "quotes.bin")
    )

    if not os.path.exists(input_path):
        print(f"ERROR: Input file not found: {input_path}", file=sys.stderr)
        sys.exit(1)

    generate(input_path, output_path)


if __name__ == "__main__":
    main()
