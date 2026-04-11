#!/usr/bin/env python3
# /// script
# requires-python = ">=3.11"
# dependencies = ["freetype-py"]
# ///
"""gen_font.py — Generate C font arrays from a TTF/OTF file for the book-clock ESP32 project.

Output files are named font_<variant>_<size>.c/.h so font_select.h can reference them
by role without touching any display code. To swap fonts, re-run this tool and rebuild.

Usage (from repo root):
    uv run tools/gen_font.py --font tools/fonts/Quicksand_Book.otf --size 36 --variant book
    uv run tools/gen_font.py --font tools/fonts/Quicksand_Bold.otf --size 36 --variant bold

Or run all sizes at once:
    tools/gen_all_fonts.sh
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import freetype

# Printable ASCII 32 (space) through 126 (~)
CHARSET = list(range(32, 127))


def clamp_i8(v: int) -> int:
    return max(-128, min(127, v))


def clamp_u8(v: int) -> int:
    return max(0, min(255, v))


def generate(font_path: Path, size_px: int, variant: str, output_dir: Path) -> None:
    face = freetype.Face(str(font_path))
    face.set_char_size(size_px * 64, 0, 72, 72)

    ascender_px = face.size.ascender >> 6

    # ── Render every glyph in the charset ─────────────────────────────────────
    glyphs: list[dict] = []
    for cp in CHARSET:
        face.load_char(
            chr(cp), freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_NORMAL
        )
        g = face.glyph
        bm = g.bitmap

        w = bm.width
        h = bm.rows
        advance = clamp_u8(g.advance.x >> 6)
        gx = clamp_i8(g.bitmap_left)
        # y offset from line-top to glyph-bitmap-top (negative = above line top)
        gy = clamp_i8(ascender_px - g.bitmap_top)

        # FreeType: 0=background, 255=ink
        # Project renderer: 0=ink, 255=background  → invert
        pixels: list[int] = []
        if w > 0 and h > 0:
            for row in range(h):
                for col in range(w):
                    v = bm.buffer[row * bm.pitch + col]
                    pixels.append(255 - v)

        glyphs.append(
            {
                "cp": cp,
                "x": gx,
                "y": gy,
                "advance": advance,
                "w": w,
                "h": h,
                "pixels": pixels,
            }
        )

    # ── Collect kerning pairs for every glyph ─────────────────────────────────
    kern_by_cp: dict[int, list[tuple[int, int]]] = {}
    for g in glyphs:
        pairs: list[tuple[int, int]] = []
        for g2 in glyphs:
            kv = face.get_kerning(
                chr(g["cp"]), chr(g2["cp"]), freetype.FT_KERNING_DEFAULT
            )
            kx = kv.x >> 6
            if kx != 0:
                pairs.append((g2["cp"], clamp_i8(kx)))
        kern_by_cp[g["cp"]] = pairs

    # ── Build flat kern array and assign per-glyph indices ────────────────────
    kern_flat: list[tuple[int, int]] = []
    for g in glyphs:
        g["kern_idx"] = len(kern_flat)
        g["kern_n"] = len(kern_by_cp[g["cp"]])
        kern_flat.extend(kern_by_cp[g["cp"]])

    # ── Build flat pixel data array ───────────────────────────────────────────
    data_flat: list[int] = [0]  # sentinel at [0] so space glyph pointer is valid
    for g in glyphs:
        if g["pixels"]:
            g["data_offset"] = len(data_flat)
            data_flat.extend(g["pixels"])
        else:
            g["data_offset"] = 0  # points to sentinel, never read (w=h=0)

    # ── Emit files ────────────────────────────────────────────────────────────
    sym = f"font_{variant}_{size_px}"
    output_dir.mkdir(parents=True, exist_ok=True)
    c_path = output_dir / f"{sym}.c"
    h_path = output_dir / f"{sym}.h"

    # Header
    h_path.write_text(
        "#pragma once\n"
        '#include "bmfont.h"\n'
        "\n"
        f"extern const font_t {sym}_font;\n"
    )
    print(f"  wrote {h_path}")

    # Source
    lines: list[str] = []
    lines.append(f'#include "{sym}.h"')
    lines.append("")

    # Pixel data array
    hex_vals = [f"0x{v:02X}" for v in data_flat]
    lines.append(f"static const uint8_t {sym}_data[{len(data_flat)}] = {{")
    for i in range(0, len(hex_vals), 16):
        chunk = ", ".join(hex_vals[i : i + 16])
        lines.append(f"    {chunk},")
    lines.append("};")
    lines.append("")

    # Kern array
    if kern_flat:
        lines.append(f"static const kern_t {sym}_kern[{len(kern_flat)}] = {{")
        for next_cp, kval in kern_flat:
            lines.append(f"    {{{next_cp}, {kval}}},")
        lines.append("};")
    else:
        lines.append(f"static const kern_t {sym}_kern[1] = {{{{0, 0}}}};")
    lines.append("")

    # Glyph array
    lines.append(f"static const glyph_t {sym}_glyph[{len(glyphs)}] = {{")
    for g in glyphs:
        data_ref = f"&{sym}_data[{g['data_offset']}]"
        lines.append(
            f"    {{{g['cp']}, {g['x']}, {g['y']}, {g['advance']}, "
            f"{g['kern_n']}, {g['kern_idx']}, {g['w']}, {g['h']}, {data_ref}}},"
        )
    lines.append("};")
    lines.append("")

    # Font struct
    lines.append(f"const font_t {sym}_font = {{")
    lines.append(f"    {ascender_px},       /* baseline: ascender height in px */")
    lines.append(f"    {len(glyphs)},       /* glyph_count */")
    lines.append("    8,               /* bpp (8-bit grayscale, inverted) */")
    lines.append(f"    {sym}_kern,")
    lines.append(f"    {sym}_glyph,")
    lines.append("};")
    lines.append("")

    c_path.write_text("\n".join(lines))
    print(f"  wrote {c_path}")
    print(
        f"  baseline={ascender_px}px  glyphs={len(glyphs)}"
        f"  kern_pairs={len(kern_flat)}  data_bytes={len(data_flat)}"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate C font arrays for the book-clock ESP32 project"
    )
    parser.add_argument("--font", required=True, help="Path to .otf/.ttf font file")
    parser.add_argument("--size", type=int, required=True, help="Font size in pixels")
    parser.add_argument(
        "--variant",
        required=True,
        help="Variant name written into the output filename (e.g. book, bold)",
    )
    parser.add_argument(
        "--output", default="main/fonts", help="Output directory (default: main/fonts)"
    )
    args = parser.parse_args()

    font_path = Path(args.font)
    if not font_path.exists():
        print(f"error: font file not found: {font_path}", file=sys.stderr)
        sys.exit(1)

    print(f"Generating font_{args.variant}_{args.size} from {font_path.name} ...")
    generate(font_path, args.size, args.variant, Path(args.output))


if __name__ == "__main__":
    main()
