#include "bmfont_renderer.h"
#include "epd/GUI_Paint.h"
#include <string.h>

/**
 * Find glyph for a given character code point
 */
static const glyph_t *find_glyph(const bmfont_t *font, uint16_t ch) {
    if (!font || !font->glyph)
        return NULL;

    // Linear search through glyphs - could be optimized with binary search
    for (uint16_t i = 0; i < font->glyph_count; i++) {
        if (font->glyph[i].cp == ch) {
            return &font->glyph[i];
        }
    }
    return NULL;
}

/**
 * Get kerning between two characters
 */
static int8_t get_kerning(const bmfont_t *font, const glyph_t *glyph, uint16_t next_ch) {
    if (!font || !glyph || !font->kern || glyph->kern_n == 0) {
        return 0;
    }

    // Check kerning pairs for this glyph
    for (uint8_t i = 0; i < glyph->kern_n; i++) {
        uint16_t kern_idx = glyph->kern_idx + i;
        if (kern_idx < 65535 && font->kern[kern_idx].cp == next_ch) { // Bounds check
            return font->kern[kern_idx].k;
        }
    }
    return 0;
}

/**
 * Draw a single pixel using the EPD library
 */
static void draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    Paint_DrawPoint(x, y, color, DOT_PIXEL_1X1, DOT_FILL_AROUND);
}

uint16_t bmfont_draw_char(uint16_t x, uint16_t y, uint16_t ch, const bmfont_t *font, uint16_t color,
                          uint16_t bgcolor) {
    if (!font)
        return 0;

    const glyph_t *glyph = find_glyph(font, ch);
    if (!glyph) {
        // Character not found, try space or return default advance
        glyph = find_glyph(font, ' ');
        if (!glyph)
            return 8;          // Default character width
        return glyph->advance; // Return advance without drawing
    }

    // Calculate drawing position with glyph offset
    int16_t draw_x = x + glyph->x;
    int16_t draw_y = y + glyph->y;

    // Draw the glyph bitmap
    const uint8_t *data = glyph->data;
    if (data && glyph->w > 0 && glyph->h > 0) {
        uint8_t bpp = font->bpp ? font->bpp : 8;

        for (uint8_t row = 0; row < glyph->h; row++) {
            for (uint8_t col = 0; col < glyph->w; col++) {
                uint16_t pixel_idx = row * glyph->w + col;
                uint8_t pixel_value;

                if (bpp == 4) {
                    // 4-bit: two pixels per byte, high nibble first
                    uint16_t byte_idx = pixel_idx / 2;
                    pixel_value = (pixel_idx & 1)
                                      ? (data[byte_idx] & 0x0F) // odd pixel: low nibble
                                      : (data[byte_idx] >> 4);  // even pixel: high nibble
                } else {
                    // 8-bit grayscale: one byte per pixel
                    pixel_value = data[pixel_idx];
                }

                // Scale to 0-255 and threshold (inverted: 0=ink, max=background)
                uint8_t alpha = (pixel_value * 255) / ((1 << bpp) - 1);
                if (alpha < 128) {
                    draw_pixel(draw_x + col, draw_y + row, color);
                } else if (bgcolor != color) {
                    draw_pixel(draw_x + col, draw_y + row, bgcolor);
                }
            }
        }
    }

    return glyph->advance;
}

uint16_t bmfont_draw_string(uint16_t x, uint16_t y, const char *str, const bmfont_t *font,
                            uint16_t color, uint16_t bgcolor) {
    if (!font || !str)
        return 0;

    uint16_t cur_x = x;
    const char *p = str;
    uint16_t prev_ch = 0;

    while (*p) {
        uint16_t ch = (uint8_t)*p; // For now, only support ASCII

        const glyph_t *glyph = find_glyph(font, ch);
        if (glyph) {
            // Apply kerning from previous character
            if (prev_ch != 0) {
                const glyph_t *prev_glyph = find_glyph(font, prev_ch);
                if (prev_glyph) {
                    cur_x += get_kerning(font, prev_glyph, ch);
                }
            }

            // Draw the character
            uint16_t advance = bmfont_draw_char(cur_x, y, ch, font, color, bgcolor);
            cur_x += advance;
        } else {
            // Character not found, advance by a default amount
            cur_x += 8;
        }

        prev_ch = ch;
        p++;
    }

    return cur_x - x;
}

uint16_t bmfont_string_width(const char *str, const bmfont_t *font) {
    if (!font || !str)
        return 0;

    uint16_t width = 0;
    const char *p = str;
    uint16_t prev_ch = 0;

    while (*p) {
        uint16_t ch = (uint8_t)*p;

        const glyph_t *glyph = find_glyph(font, ch);
        if (glyph) {
            // Apply kerning from previous character
            if (prev_ch != 0) {
                const glyph_t *prev_glyph = find_glyph(font, prev_ch);
                if (prev_glyph) {
                    width += get_kerning(font, prev_glyph, ch);
                }
            }

            width += glyph->advance;
        } else {
            width += 8; // Default character width
        }

        prev_ch = ch;
        p++;
    }

    return width;
}

uint16_t bmfont_height(const bmfont_t *font) {
    if (!font)
        return 0;
    return font->baseline;
}