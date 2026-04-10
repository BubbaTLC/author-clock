#ifndef WIFI_STA_H
#define WIFI_STA_H

#include "esp_err.h"
#include <stdbool.h>

/**
 * @brief Connect to WiFi network in STA mode
 *
 * Attempts to connect to the specified WiFi network with retries.
 * Uses event handlers for connection status and creates event group
 * to coordinate connection attempts.
 *
 * @param ssid WiFi network SSID (max 32 chars + null terminator)
 * @param password WiFi network password (max 64 chars + null terminator)
 * @return esp_err_t ESP_OK on successful connection, ESP_FAIL on failure
 */
esp_err_t author_wifi_sta_connect(const char *ssid, const char *password);

/**
 * @brief Disconnect from WiFi network
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t author_wifi_sta_disconnect(void);

/**
 * @brief Check if WiFi STA is currently connected
 *
 * @return true if connected and has IP address, false otherwise
 */
bool author_wifi_sta_is_connected(void);

#endif // WIFI_STA_H