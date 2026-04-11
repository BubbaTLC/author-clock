#ifndef WEATHER_ICONS_H
#define WEATHER_ICONS_H

/**
 * @file weather_icons.h
 * @brief Simple 1-bit weather icon drawing using GUI_Paint primitives.
 *
 * Icons are 32x32 pixels drawn at the given (x, y) top-left coordinate.
 * Call weather_icon_draw() with an OWM weather ID to pick and render the
 * appropriate icon.
 *
 * OWM ID ranges:
 *   200–232  → thunderstorm
 *   300–531  → rain / drizzle
 *   600–622  → snow
 *   700–781  → fog / mist
 *   800      → clear (sun)
 *   801–804  → clouds
 */

#include <stdint.h>
#include "epd/GUI_Paint.h"

#define ICON_SIZE 32

/**
 * @brief Draw a weather icon at (x, y) based on OWM weather ID.
 * @param x        Left edge of the icon
 * @param y        Top edge of the icon
 * @param weather_id  OWM condition code from weather[0].id
 */
static inline void weather_icon_draw(uint16_t x, uint16_t y, int weather_id) {
    uint16_t cx = x + ICON_SIZE / 2;
    uint16_t cy = y + ICON_SIZE / 2;

    if (weather_id == 800) {
        // ── Clear / Sun ──────────────────────────────────────────────────────
        // Central filled circle (sun body, r=7)
        Paint_DrawCircle(cx, cy, 7, BLACK, DOT_PIXEL_2X2, DRAW_FILL_FULL);
        // 8 rays of length 4 starting at r=10
        uint16_t r0 = 10, r1 = 14;
        // Cardinal directions
        Paint_DrawLine(cx, cy - r0, cx, cy - r1, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(cx, cy + r0, cx, cy + r1, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(cx - r0, cy, cx - r1, cy, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(cx + r0, cy, cx + r1, cy, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        // Diagonal rays (approx 45°, offset ~7px at r=10)
        uint16_t d = 7;
        Paint_DrawLine(cx - d, cy - d, cx - d - 3, cy - d - 3, BLACK, DOT_PIXEL_2X2,
                       LINE_STYLE_SOLID);
        Paint_DrawLine(cx + d, cy - d, cx + d + 3, cy - d - 3, BLACK, DOT_PIXEL_2X2,
                       LINE_STYLE_SOLID);
        Paint_DrawLine(cx - d, cy + d, cx - d - 3, cy + d + 3, BLACK, DOT_PIXEL_2X2,
                       LINE_STYLE_SOLID);
        Paint_DrawLine(cx + d, cy + d, cx + d + 3, cy + d + 3, BLACK, DOT_PIXEL_2X2,
                       LINE_STYLE_SOLID);

    } else if (weather_id >= 801 && weather_id <= 804) {
        // ── Clouds ────────────────────────────────────────────────────────────
        // Main cloud body: two overlapping circles + rectangle base
        Paint_DrawCircle(cx - 5, cy + 2, 8, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawCircle(cx + 5, cy + 2, 6, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawCircle(cx, cy - 2, 9, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        // Fill the bottom gap
        Paint_DrawRectangle(x + 4, cy + 2, x + ICON_SIZE - 4, y + ICON_SIZE - 2, BLACK,
                            DOT_PIXEL_1X1, DRAW_FILL_FULL);

    } else if (weather_id >= 300 && weather_id <= 531) {
        // ── Rain ──────────────────────────────────────────────────────────────
        // Cloud (smaller, shifted up)
        uint16_t cloud_cy = cy - 5;
        Paint_DrawCircle(cx - 4, cloud_cy, 6, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawCircle(cx + 4, cloud_cy, 5, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawCircle(cx, cloud_cy - 3, 7, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawRectangle(x + 5, cloud_cy, x + ICON_SIZE - 5, cloud_cy + 7, BLACK, DOT_PIXEL_1X1,
                            DRAW_FILL_FULL);
        // Rain drops: 3 short diagonal lines
        uint16_t drop_y = cloud_cy + 10;
        Paint_DrawLine(cx - 6, drop_y, cx - 9, drop_y + 6, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(cx, drop_y, cx - 3, drop_y + 6, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(cx + 6, drop_y, cx + 3, drop_y + 6, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);

    } else if (weather_id >= 600 && weather_id <= 622) {
        // ── Snow ──────────────────────────────────────────────────────────────
        // Cloud (smaller, shifted up)
        uint16_t cloud_cy = cy - 5;
        Paint_DrawCircle(cx - 4, cloud_cy, 6, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawCircle(cx + 4, cloud_cy, 5, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawCircle(cx, cloud_cy - 3, 7, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawRectangle(x + 5, cloud_cy, x + ICON_SIZE - 5, cloud_cy + 7, BLACK, DOT_PIXEL_1X1,
                            DRAW_FILL_FULL);
        // Snowflakes: 3 small asterisks (cross + diagonal)
        uint16_t sf_y = cloud_cy + 12;
        uint8_t positions[3][2] = {{cx - 7, sf_y}, {cx, sf_y + 2}, {cx + 7, sf_y}};
        for (int i = 0; i < 3; i++) {
            uint16_t sx = positions[i][0], sy = positions[i][1];
            Paint_DrawLine(sx - 3, sy, sx + 3, sy, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
            Paint_DrawLine(sx, sy - 3, sx, sy + 3, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
            Paint_DrawLine(sx - 2, sy - 2, sx + 2, sy + 2, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
            Paint_DrawLine(sx + 2, sy - 2, sx - 2, sy + 2, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        }

    } else if (weather_id >= 200 && weather_id <= 232) {
        // ── Thunderstorm ──────────────────────────────────────────────────────
        // Cloud (smaller, shifted up)
        uint16_t cloud_cy = cy - 6;
        Paint_DrawCircle(cx - 4, cloud_cy, 6, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawCircle(cx + 4, cloud_cy, 5, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawCircle(cx, cloud_cy - 3, 7, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawRectangle(x + 5, cloud_cy, x + ICON_SIZE - 5, cloud_cy + 7, BLACK, DOT_PIXEL_1X1,
                            DRAW_FILL_FULL);
        // Lightning bolt: thick diagonal lines forming a zig-zag
        uint16_t bolt_top = cloud_cy + 9;
        Paint_DrawLine(cx + 3, bolt_top, cx - 2, bolt_top + 7, BLACK, DOT_PIXEL_2X2,
                       LINE_STYLE_SOLID);
        Paint_DrawLine(cx - 2, bolt_top + 7, cx + 2, bolt_top + 7, BLACK, DOT_PIXEL_2X2,
                       LINE_STYLE_SOLID);
        Paint_DrawLine(cx + 2, bolt_top + 7, cx - 3, bolt_top + 14, BLACK, DOT_PIXEL_2X2,
                       LINE_STYLE_SOLID);

    } else {
        // ── Fog / Mist / Atmosphere (700–781) or unknown ─────────────────────
        // Four horizontal dashed lines of varying width
        uint8_t line_y_offsets[4] = {6, 13, 20, 27};
        uint8_t line_widths[4] = {20, 24, 22, 18};
        for (int i = 0; i < 4; i++) {
            uint16_t lx = cx - line_widths[i] / 2;
            uint16_t ly = y + line_y_offsets[i];
            Paint_DrawLine(lx, ly, lx + line_widths[i], ly, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        }
    }
}

#endif // WEATHER_ICONS_H
