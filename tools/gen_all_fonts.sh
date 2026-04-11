#!/usr/bin/env bash
# gen_all_fonts.sh — Generate all 6 font files for the book-clock project.
#
# Run from the repo root:
#   tools/gen_all_fonts.sh
#
# Output: main/fonts/font_{book,bold}_{24,36,48}.c/.h
# To swap the active font sizes, edit main/fonts/font_select.h — no C code changes needed.
#
# --embolden 32: synthetic outline thickening for the thin Book face (~+1px stroke width).
# Bold is already heavy enough; leave it at 0.

set -e
cd "$(dirname "$0")/.."

BOOK="tools/fonts/Quicksand_Book.otf"
BOLD="tools/fonts/Quicksand_Bold.otf"

for SIZE in 24 36 48; do
    echo "── book ${SIZE}px ──────────────────────────────────────"
    uv run tools/gen_font.py --font "$BOOK" --size "$SIZE" --variant book --embolden 32

    echo "── bold ${SIZE}px ──────────────────────────────────────"
    uv run tools/gen_font.py --font "$BOLD" --size "$SIZE" --variant bold
done

echo ""
echo "Done. Edit main/fonts/font_select.h to change active font sizes."
