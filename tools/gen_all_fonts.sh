#!/usr/bin/env bash
# gen_all_fonts.sh — Generate all 6 font files for the book-clock project.
#
# Run from the repo root:
#   tools/gen_all_fonts.sh
#
# Output: main/fonts/font_{book,bold}_{24,36,48}.c/.h
# To swap the active font sizes, edit main/fonts/font_select.h — no C code changes needed.
#
# Current typeface: Roboto Condensed (regular + bold).
# To switch fonts, update the BOOK/BOLD paths below and re-run.
# Use --embolden N (e.g. 32) for thin faces like Quicksand Book that look jaggy on e-paper.

set -e
cd "$(dirname "$0")/.."

BOOK="tools/fonts/Roboto-Condensed.ttf"
BOLD="tools/fonts/Roboto-BoldCondensed.ttf"

for SIZE in 24 36 48; do
    echo "── book ${SIZE}px ──────────────────────────────────────"
    uv run tools/gen_font.py --font "$BOOK" --size "$SIZE" --variant book --bpp 4 --dpi 125

    echo "── bold ${SIZE}px ──────────────────────────────────────"
    uv run tools/gen_font.py --font "$BOLD" --size "$SIZE" --variant bold --bpp 4 --dpi 125
done

echo ""
echo "Done. Edit main/fonts/font_select.h to change active font sizes."
