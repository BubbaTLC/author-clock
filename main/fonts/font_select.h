#pragma once
/*
 * font_select.h — Font role assignments for the book-clock display.
 *
 * Change the #include and #define lines below to swap fonts project-wide.
 * No other C files need to be modified — just rebuild after changing this file.
 *
 * Generated font files available:
 *   font_book_24  font_book_36  font_book_48
 *   font_bold_24  font_bold_36  font_bold_48
 *
 * Current typeface: Roboto Condensed (regular + bold).
 *
 * Regenerate any font with (from repo root):
 *   uv run tools/gen_font.py --font tools/fonts/Roboto-Condensed.ttf --size 36 --variant book
 * Or regenerate all six at once:
 *   tools/gen_all_fonts.sh
 */

#include "font_book_24.h"
#include "font_book_36.h"
#include "font_book_48.h"
#include "font_bold_24.h"
#include "font_bold_36.h"
#include "font_bold_48.h"

/* Quote body text */
#define font_body font_book_36_font

/* Time-string highlight within the quote (bolder weight) */
#define font_body_bold font_bold_36_font

/* Book title / author attribution line */
#define font_attrib font_book_24_font

/* UI screens: time/date header, provisioning, error messages */
#define font_ui font_book_24_font
