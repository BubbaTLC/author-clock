#pragma once
#include <stdint.h>

// Reference-only struct definitions. Provide your own real declarations.

// Glyph descriptor
typedef struct {
    uint16_t cp;         // code-point
    int8_t x, y;         // draw offset
    uint8_t advance;     // advance
    uint8_t kern_n;      // kerning pairs
    uint16_t kern_idx;   // kerning data index
    uint8_t w, h;        // size
    const uint8_t *data; // data pointer
} glyph_t;

typedef struct {
    uint16_t cp; // code-point
    int8_t k;    // kerning
} kern_t;

typedef struct {
    uint16_t baseline;
    uint16_t glyph_count;
    uint16_t bpp;
    const kern_t *kern;
    const glyph_t *glyph;
} font_t;

// Type alias for consistency with renderer functions
typedef font_t bmfont_t;
