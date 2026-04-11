#ifndef BMFONT_RENDERER_H
#define BMFONT_RENDERER_H

#include <stdint.h>
#include "fonts/bmfont.h"

/**
 * @brief Render a single character using bmfont format
 * @param x X position to draw at
 * @param y Y position to draw at
 * @param ch Character to draw
 * @param font bmfont font to use
 * @param color Foreground color
 * @param bgcolor Background color
 * @return Width of the drawn character, or 0 if character not found
 */
uint16_t bmfont_draw_char(uint16_t x, uint16_t y, uint16_t ch, const bmfont_t *font, uint16_t color,
                          uint16_t bgcolor);

/**
 * @brief Render a string using bmfont format
 * @param x X position to start drawing at
 * @param y Y position to draw at
 * @param str String to draw
 * @param font bmfont font to use
 * @param color Foreground color
 * @param bgcolor Background color
 * @return Width of the rendered string in pixels
 */
uint16_t bmfont_draw_string(uint16_t x, uint16_t y, const char *str, const bmfont_t *font,
                            uint16_t color, uint16_t bgcolor);

/**
 * @brief Get the width of a string if rendered with the given bmfont
 * @param str String to measure
 * @param font bmfont font to use
 * @return Width in pixels
 */
uint16_t bmfont_string_width(const char *str, const bmfont_t *font);

/**
 * @brief Get the height of a bmfont
 * @param font bmfont font
 * @return Height in pixels
 */
uint16_t bmfont_height(const bmfont_t *font);

#endif // BMFONT_RENDERER_H