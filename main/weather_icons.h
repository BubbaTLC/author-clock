#ifndef WEATHER_ICONS_H
#define WEATHER_ICONS_H

/**
 * @file weather_icons.h
 * @brief Render pre-baked OWM weather icons onto the e-paper display.
 *
 * Bitmaps are 1-bit packed arrays generated offline by tools/gen_weather_icons.py
 * from the official OWM PNG icons.  See weather_icons_bmp.h for the raw data.
 *
 * Call weather_icon_draw() with the OWM icon code (e.g. "01d") and/or weather_id.
 * The icon code takes priority; weather_id is used as a fallback when the code is
 * not yet available (e.g. before the first weather fetch completes).
 */

#include <stdint.h>
#include <string.h>
#include "epd/GUI_Paint.h"
#include "weather_icons_bmp.h"

// ICON_SIZE exposed for layout code (matches WEATHER_ICON_W from the generated header)
#define ICON_SIZE WEATHER_ICON_W

// Map an OWM weather_id to a representative day-icon code for the fallback path.
static inline const char *weather_id_to_icon(int weather_id) {
    if (weather_id == 800)
        return "01d";
    if (weather_id >= 801 && weather_id <= 804)
        return "03d";
    if (weather_id >= 300 && weather_id <= 531)
        return "10d";
    if (weather_id >= 600 && weather_id <= 622)
        return "13d";
    if (weather_id >= 200 && weather_id <= 232)
        return "11d";
    return "50d"; // fog / mist / atmosphere / unknown
}

/**
 * @brief Draw a weather icon at (x, y).
 *
 * @param x          Left edge of the icon
 * @param y          Top edge of the icon
 * @param icon_code  OWM icon string from weather[0].icon, e.g. "01d".
 *                   May be NULL or empty — falls back to weather_id.
 * @param weather_id OWM condition code from weather[0].id (fallback only)
 */
static inline void weather_icon_draw(uint16_t x, uint16_t y, const char *icon_code,
                                     int weather_id) {
    const uint8_t *bmp = weather_icon_bitmap(icon_code);
    if (!bmp) {
        bmp = weather_icon_bitmap(weather_id_to_icon(weather_id));
    }
    if (!bmp)
        return;

    for (int row = 0; row < WEATHER_ICON_H; row++) {
        for (int col = 0; col < WEATHER_ICON_W; col++) {
            if (bmp[row * WEATHER_ICON_BYTES_PER_ROW + col / 8] & (0x80 >> (col % 8))) {
                Paint_SetPixel(x + (uint16_t)col, y + (uint16_t)row, BLACK);
            }
        }
    }
}

#endif // WEATHER_ICONS_H
