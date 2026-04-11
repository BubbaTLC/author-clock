---
name: gen-fonts
description: "Generate C font arrays for the book-clock ESP32 e-paper display. Use when: adding a new font, regenerating fonts after changing size or weight, troubleshooting blank text on display, changing the active font role (body, bold, attribution, UI), adding a new typeface variant, or adjusting emboldening. Covers gen_font.py usage, gen_all_fonts.sh, font_select.h role assignments, and bmfont.h data types."
argument-hint: "Describe the font change: e.g. 'regenerate all fonts', 'add size 60', 'swap body font to bold 48'"
---

# Font Generation for book-clock

## Overview

Font bitmaps are pre-baked into C arrays at build time using `tools/gen_font.py`.
The output lives in `main/fonts/` and is referenced by `main/fonts/font_select.h`.
No display code needs to change when swapping fonts — only `font_select.h` and a rebuild.

## File Map

| File | Purpose |
|------|---------|
| `tools/gen_font.py` | Renders glyphs with FreeType, writes `.c`/`.h` pairs |
| `tools/gen_all_fonts.sh` | Runs gen_font.py for all 6 combinations (2 variants × 3 sizes) |
| `tools/fonts/` | Source `.otf`/`.ttf` files |
| `main/fonts/font_{variant}_{size}.c/.h` | Generated output (committed) |
| `main/fonts/font_select.h` | Maps role macros (`font_body`, `font_body_bold`, etc.) to generated fonts |
| `main/fonts/bmfont.h` | C structs: `font_t`, `glyph_t`, `kern_t` |

## Available Fonts (defaults)

| File | Variant | Sizes |
|------|---------|-------|
| `tools/fonts/Quicksand_Book.otf` | `book` | 24, 36, 48 |
| `tools/fonts/Quicksand_Bold.otf` | `bold` | 24, 36, 48 |

## Font Role Assignments (`font_select.h`)

| Macro | Default | Usage |
|-------|---------|-------|
| `font_body` | `font_book_36_font` | Quote body text |
| `font_body_bold` | `font_bold_36_font` | Time-string highlight within quote |
| `font_attrib` | `font_book_24_font` | Book title / author attribution |
| `font_ui` | `font_book_24_font` | Time header, provisioning, error screens |

## Procedures

### Regenerate All 6 Default Fonts

Run from the repo root:

```bash
tools/gen_all_fonts.sh
```

This generates `font_book_{24,36,48}` and `font_bold_{24,36,48}` in `main/fonts/`.

### Regenerate a Single Font

```bash
# Book face — always pass --embolden 32 (thin face needs synthetic thickening)
uv run tools/gen_font.py --font tools/fonts/Quicksand_Book.otf --size 36 --variant book --embolden 32

# Bold face — no embolden needed
uv run tools/gen_font.py --font tools/fonts/Quicksand_Bold.otf --size 36 --variant bold
```

### Add a New Size

1. Run gen_font.py with your desired `--size` (e.g. `--size 60`).
2. New files appear: `main/fonts/font_book_60.c/.h` and `main/fonts/font_bold_60.c/.h`.
3. Update `font_select.h`: add the `#include` lines and change the relevant `#define`.
4. Rebuild the project.

### Add a New Typeface

1. Drop the `.otf`/`.ttf` file into `tools/fonts/`.
2. Run gen_font.py with a new `--variant` name (e.g. `--variant serif`).
3. Include the generated `.h` in `font_select.h` and assign to a role macro.

### Swap Active Font Size (no regeneration needed)

Edit `main/fonts/font_select.h` — change `#define font_body` to a different already-generated symbol, then rebuild. No source files need touching.

## Parameter Reference

| Argument | Required | Description |
|----------|----------|-------------|
| `--font` | Yes | Path to `.otf`/`.ttf` source file |
| `--size` | Yes | Pixel height (72 DPI basis). Typical: 24, 36, 48 |
| `--variant` | Yes | Name embedded in output filename (e.g. `book`, `bold`) |
| `--embolden` | No | Synthetic outline thickening in 1/64-px units. Default: 0. Use 32 for thin faces (+1px stroke). Range: 16–48 |
| `--output` | No | Output directory. Default: `main/fonts` |

## Data Format

- **Charset**: printable ASCII 32 (space) – 126 (`~`), 95 glyphs total.
- **Pixel encoding**: 8-bit grayscale, **inverted** (0 = ink, 255 = background) to match the e-paper renderer.
- **Kerning**: full pair table extracted from the font; stored as `kern_t[]` in the `.c` file.
- **Struct layout** (from `bmfont.h`):
  ```c
  typedef struct { uint16_t cp; int8_t x, y; uint8_t advance; uint8_t kern_n; uint16_t kern_idx; uint8_t w, h; const uint8_t *data; } glyph_t;
  typedef struct { uint16_t cp; int8_t k; } kern_t;
  typedef struct { uint16_t baseline; uint16_t glyph_count; uint16_t bpp; const kern_t *kern; const glyph_t *glyph; } font_t;
  ```

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| Text renders blank / all-white | Pixels not inverted OR wrong font pointer | Verify `--embolden` didn't produce zero-width glyphs; check struct field order |
| `uv: command not found` | `uv` not installed | Run `pip install uv` or `brew install uv` |
| `freetype-py` import error | uv inline deps not resolved | Ensure you're running via `uv run tools/gen_font.py`, not `python tools/gen_font.py` |
| Font file not found | `.otf` missing from `tools/fonts/` | Ensure the font file exists in `tools/fonts/` |
| Text too thin on e-paper | Book face with `--embolden 0` | Always pass `--embolden 32` for Book variant |
| CMake build error after adding font | New `.c` not in build system | `main/CMakeLists.txt` uses glob — just rebuild, CMake will pick up the new file |

## Prerequisites

- `uv` package manager installed (provides `freetype-py` automatically via inline script metadata)
- Source `.otf`/`.ttf` files in `tools/fonts/`
- Run all commands from the **repo root** (`/Users/bubbachabot/3d_printing_projects/book-clock`)
