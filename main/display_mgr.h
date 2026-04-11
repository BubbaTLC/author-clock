#ifndef DISPLAY_MGR_H
#define DISPLAY_MGR_H

#include <stdint.h>
#include <stdbool.h>
#include "quote_types.h"
#include "weather_fetcher.h"

/**
 * @brief Initialize the e-paper display
 * Calls DEV_Module_Init, allocates image buffer, EPD_7IN5_V2_Init, EPD_7IN5_V2_Clear.
 * Must be called before any other display function.
 * @return 0 on success, non-zero on failure
 */
int display_mgr_init(void);

/**
 * @brief Show the WiFi provisioning screen
 * @param ap_ssid  AP network name to connect to
 * @param ip_str   URL to visit (e.g. "192.168.4.69")
 */
void display_mgr_show_provisioning(const char *ap_ssid, const char *ip_str);

/**
 * @brief Show a "Connecting to <ssid>..." screen
 */
void display_mgr_show_connecting(const char *ssid);

/**
 * @brief Show a full-screen error message
 */
void display_mgr_show_error(const char *msg);

/**
 * @brief Render the main clock face
 *
 * Reads display preferences (date mode, weather visibility, temp unit) from NVS.
 * - Top-left: date/time according to date_mode setting.
 * - Top-right: weather icon + temp + condition when weather is enabled.
 * - Horizontal rule: drawn only when top bar has any content.
 * - Quote: vertically centered in the available area (below rule, or full display).
 *
 * @param time_h    Current hour (0-23)
 * @param time_m    Current minute (0-59)
 * @param date_str  Human-readable date string (e.g. "Tuesday, April 9")
 * @param quote     Quote to display (may be NULL to show placeholder)
 * @param weather   Current weather data (may be NULL if not available/disabled)
 */
void display_mgr_update(uint8_t time_h, uint8_t time_m, const char *date_str,
                        const quote_result_t *quote, const weather_data_t *weather);

#endif // DISPLAY_MGR_H
