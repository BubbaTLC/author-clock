#!/usr/bin/env python3
# /// script
# requires-python = ">=3.11"
# dependencies = ["freetype-py", "pillow"]
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
import ctypes
import sys
from pathlib import Path

import freetype

# Printable ASCII 32 (space) through 126 (~)
CHARSET = list(range(32, 127))


def clamp_i8(v: int) -> int:
    return max(-128, min(127, v))


def clamp_u8(v: int) -> int:
    return max(0, min(255, v))


def _pack_4bpp(pixels: list[int]) -> list[int]:
    """Pack 8-bit inverted pixel values into 4-bit nibbles (2 per byte, high nibble first).

    Input values are 0=ink, 255=background (8-bit inverted).
    Each value is quantized to 4 bits: v4 = round(v * 15 / 255), clamped 0-15.
    Pairs are packed as (v4_even << 4) | v4_odd.  Odd-length input is padded with 0.
    """
    out: list[int] = []
    for i in range(0, len(pixels), 2):
        hi = round(pixels[i] * 15 / 255)
        lo = round(pixels[i + 1] * 15 / 255) if i + 1 < len(pixels) else 0
        out.append(((hi & 0xF) << 4) | (lo & 0xF))
    return out


def render_preview(
    glyphs: list[dict],
    kern_by_cp: dict[int, list[tuple[int, int]]],
    ascender_px: int,
    size_px: int,
    sym: str,
    output_dir: Path,
    preview_text: str | None = None,
) -> None:
    from PIL import Image

    W, H = 800, 480
    BG = 220  # light gray ~= e-paper surface

    img = Image.new("L", (W, H), BG)
    pix = img.load()

    glyph_map = {g["cp"]: g for g in glyphs}

    def draw_line(text: str, x: int, y: int) -> None:
        cur_x = x
        prev_cp: int | None = None
        for ch in text:
            cp = ord(ch)
            g = glyph_map.get(cp)
            if g is None:
                cur_x += size_px // 3
                prev_cp = None
                continue
            # Apply kerning from previous character
            if prev_cp is not None:
                for next_cp, kv in kern_by_cp.get(prev_cp, []):
                    if next_cp == cp:
                        cur_x += kv
                        break
            # Blit glyph pixels; value 0 = ink, 255 = background
            draw_x = cur_x + g["x"]
            draw_y = y + g["y"]
            for row in range(g["h"]):
                for col in range(g["w"]):
                    v = g["pixels"][row * g["w"] + col]
                    px, py = draw_x + col, draw_y + row
                    if 0 <= px < W and 0 <= py < H:
                        # Blend toward black (0) for ink, keep BG for background
                        pix[px, py] = int(v * BG // 255)
            cur_x += g["advance"]
            prev_cp = cp

    if preview_text:
        lines = preview_text.replace("\\n", "\n").split("\n")
    else:
        lines = [
            "The quick brown fox jumps over the lazy dog.",
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ  0123456789",
            "abcdefghijklmnopqrstuvwxyz  !\"#$%&'()*+,-./:;<=>?@",
        ]

    margin = 24
    line_h = ascender_px + size_px // 4
    y = margin
    for line in lines:
        if y + line_h > H:
            break
        draw_line(line, margin, y)
        y += line_h

    out_path = output_dir / f"{sym}_preview.png"
    img.save(out_path)
    print(f"  wrote {out_path}")


def generate(
    font_path: Path,
    size_px: int,
    variant: str,
    output_dir: Path,
    embolden: int = 0,
    bpp: int = 8,
    dpi: int = 72,
    preview: bool = False,
    preview_text: str | None = None,
) -> None:
    face = freetype.Face(str(font_path))
    # Keep --size as rendered pixels regardless of DPI: point_size = pixels * 72 / dpi
    face.set_char_size(int(size_px * 72 / dpi * 64), 0, dpi, dpi)

    ascender_px = face.size.ascender >> 6

    # ── Render every glyph in the charset ─────────────────────────────────────
    glyphs: list[dict] = []
    for cp in CHARSET:
        if embolden > 0:
            # Load outline only, embolden it, then rasterize.
            # FT_Outline_Embolden expands the outline by `embolden` units (26.6 fixed-point)
            # on each side. The advance is NOT updated automatically, so we add it manually.
            face.load_char(
                chr(cp), freetype.FT_LOAD_DEFAULT | freetype.FT_LOAD_NO_BITMAP
            )
            freetype.raw.FT_Outline_Embolden(
                ctypes.byref(face.glyph.outline._FT_Outline), embolden
            )
            face.glyph.render(freetype.FT_RENDER_MODE_NORMAL)
            # Each side grows by embolden/64 px; total extra advance = ceil(embolden/32)
            extra_adv = (embolden + 31) // 32
        else:
            face.load_char(
                chr(cp), freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_NORMAL
            )
            extra_adv = 0

        g = face.glyph
        bm = g.bitmap

        w = bm.width
        h = bm.rows
        advance = clamp_u8((g.advance.x >> 6) + extra_adv)
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
            if bpp == 4:
                data_flat.extend(_pack_4bpp(g["pixels"]))
            else:
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
    lines.append(
        f"    {bpp},               /* bpp (inverted: 0=ink, max=background) */"
    )
    lines.append(f"    {sym}_kern,")
    lines.append(f"    {sym}_glyph,")
    lines.append("};")
    lines.append("")

    c_path.write_text("\n".join(lines))
    print(f"  wrote {c_path}")
    print(
        f"  baseline={ascender_px}px  glyphs={len(glyphs)}"
        f"  kern_pairs={len(kern_flat)}  data_bytes={len(data_flat)}  dpi={dpi}"
    )

    if preview:
        render_preview(
            glyphs, kern_by_cp, ascender_px, size_px, sym, output_dir, preview_text
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
        "--embolden",
        type=int,
        default=0,
        metavar="N",
        help=(
            "Synthetic outline emboldening strength in 1/64-pixel units "
            "(default: 0 = off). Values 16–48 are typical; 32 ≈ +1px stroke width. "
            "Use for thin fonts that look jaggy on e-paper."
        ),
    )
    parser.add_argument(
        "--output", default="main/fonts", help="Output directory (default: main/fonts)"
    )
    parser.add_argument(
        "--bpp",
        type=int,
        choices=[4, 8],
        default=8,
        help="Bits per pixel in the output data array: 8 (default) or 4. "
        "4bpp packs two pixels per byte (high nibble first) at 16 gray levels, "
        "halving data size. Suitable for e-paper 4-gray mode.",
    )
    parser.add_argument(
        "--dpi",
        type=int,
        default=72,
        metavar="N",
        help=(
            "Screen DPI used for FreeType hinting (default: 72). "
            "Set to 125 to match the 7.5-inch e-paper display. "
            "--size remains the rendered pixel height regardless of DPI."
        ),
    )
    parser.add_argument(
        "--preview",
        action="store_true",
        help="Write a PNG specimen sheet alongside the .c/.h files",
    )
    parser.add_argument(
        "--preview-text",
        default=None,
        metavar="TEXT",
        help=(
            "Custom preview text; use \\n to separate lines. "
            "Default: built-in pangram + alphabet specimen."
        ),
    )
    args = parser.parse_args()

    font_path = Path(args.font)
    if not font_path.exists():
        print(f"error: font file not found: {font_path}", file=sys.stderr)
        sys.exit(1)

    print(f"Generating font_{args.variant}_{args.size} from {font_path.name} ...")
    generate(
        font_path,
        args.size,
        args.variant,
        Path(args.output),
        args.embolden,
        bpp=args.bpp,
        dpi=args.dpi,
        preview=args.preview,
        preview_text=args.preview_text,
    )


if __name__ == "__main__":
    main()
