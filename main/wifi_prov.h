#ifndef WIFI_PROV_H
#define WIFI_PROV_H

#include "esp_err.h"

// Callback function type for when provisioning is complete
typedef void (*wifi_prov_complete_cb_t)(void);

/**
 * @brief Start WiFi provisioning mode
 *
 * Creates an open WiFi AP with SSID "AuthorClock" and starts HTTP server
 * on 192.168.4.69 with a web form for WiFi credentials and location settings.
 *
 * @param on_complete_cb Callback function called when provisioning is complete
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_prov_start(wifi_prov_complete_cb_t on_complete_cb);

/**
 * @brief Stop WiFi provisioning mode
 *
 * Stops HTTP server and WiFi AP mode
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_prov_stop(void);

/**
 * @brief Check if provisioning mode is currently active
 *
 * @return true if provisioning is running, false otherwise
 */
bool wifi_prov_is_active(void);

#endif // WIFI_PROV_H