#!/usr/bin/env python3
"""Validates an ESP32 partitions.csv for a 4MB flash chip."""

import sys
import argparse

FLASH_SIZE = 0x400000  # 4MB


def parse_int(value):
    value = value.strip()
    if value.startswith("0x") or value.startswith("0X"):
        return int(value, 16)
    return int(value)


def main():
    parser = argparse.ArgumentParser(description="Validate ESP32 partitions.csv")
    parser.add_argument(
        "csv",
        nargs="?",
        default="partitions.csv",
        help="Path to partitions.csv (default: partitions.csv)",
    )
    args = parser.parse_args()

    try:
        with open(args.csv) as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"ERROR: File not found: {args.csv}")
        sys.exit(1)

    partitions = []
    for lineno, line in enumerate(lines, 1):
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        parts = [p.strip() for p in stripped.split(",")]
        if len(parts) < 5:
            print(f"WARNING: Skipping malformed line {lineno}: {stripped}")
            continue
        name = parts[0]
        ptype = parts[1]
        subtype = parts[2]
        try:
            offset = parse_int(parts[3])
            size = parse_int(parts[4])
        except ValueError as e:
            print(f"WARNING: Skipping line {lineno} due to parse error: {e}")
            continue
        partitions.append(
            {
                "name": name,
                "type": ptype,
                "subtype": subtype,
                "offset": offset,
                "size": size,
                "end": offset + size,
            }
        )

    print(f"{'Name':<12} {'Offset':>10} {'Size':>10} {'End':>10}")
    print("-" * 46)
    for p in partitions:
        print(f"{p['name']:<12} {p['offset']:#010x} {p['size']:#010x} {p['end']:#010x}")
    print()

    errors = []

    # (a) All partitions fit within 4MB
    for p in partitions:
        if p["end"] > FLASH_SIZE:
            errors.append(
                f"Partition '{p['name']}' exceeds 4MB flash: "
                f"end={p['end']:#010x} > {FLASH_SIZE:#010x}"
            )

    # (b) No overlapping partitions
    for i, a in enumerate(partitions):
        for b in partitions[i + 1 :]:
            if a["offset"] < b["end"] and b["offset"] < a["end"]:
                errors.append(
                    f"Partitions '{a['name']}' and '{b['name']}' overlap: "
                    f"{a['name']}=[{a['offset']:#x},{a['end']:#x}) "
                    f"{b['name']}=[{b['offset']:#x},{b['end']:#x})"
                )

    # (c) At least one NVS partition
    has_nvs = any(p["type"] == "data" and p["subtype"] == "nvs" for p in partitions)
    if not has_nvs:
        errors.append("No NVS partition found (type=data, subtype=nvs)")

    # (d) At least one factory/app partition
    has_app = any(p["type"] == "app" or p["subtype"] == "factory" for p in partitions)
    if not has_app:
        errors.append("No factory/app partition found (type=app)")

    if errors:
        print("FAIL — errors found:")
        for err in errors:
            print(f"  ERROR: {err}")
        sys.exit(1)
    else:
        print("PASS — all partition checks passed.")


if __name__ == "__main__":
    main()
