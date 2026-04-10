#ifndef NVS_CONFIG_H
#define NVS_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

/**
 * NVS Configuration Module
 * Handles persistent storage of WiFi credentials and location settings
 * Uses NVS namespace "clock_cfg"
 */

// Maximum sizes for stored strings
#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64
#define MAX_CITY_LEN 63

/**
 * Initialize the NVS configuration system
 * Must be called before any other nvs_config functions
 * @return ESP_OK on success
 */
esp_err_t nvs_config_init(void);

/**
 * Check if WiFi credentials are stored and valid
 * @return true if SSID exists and is non-empty, false otherwise
 */
bool nvs_config_has_wifi(void);

/**
 * Save WiFi credentials to NVS
 * @param ssid WiFi network name (max 32 chars)
 * @param password WiFi password (max 64 chars)
 * @return ESP_OK on success
 */
esp_err_t nvs_config_save_wifi(const char *ssid, const char *password);

/**
 * Load WiFi credentials from NVS
 * @param ssid Buffer to store SSID (must be at least 33 bytes)
 * @param password Buffer to store password (must be at least 65 bytes)
 * @return ESP_OK on success, ESP_ERR_NVS_NOT_FOUND if no credentials stored
 */
esp_err_t nvs_config_load_wifi(char *ssid, char *password);

/**
 * Save location settings to NVS
 * @param city City name for weather lookup (max 63 chars)
 * @param tz_offset_hours Timezone offset from UTC (-12 to +14)
 * @return ESP_OK on success
 */
esp_err_t nvs_config_save_location(const char *city, int8_t tz_offset_hours);

/**
 * Load location settings from NVS
 * @param city Buffer to store city name (must be at least 64 bytes)
 * @param tz_offset_hours Pointer to store timezone offset
 * @return ESP_OK on success, ESP_ERR_NVS_NOT_FOUND if no location stored
 */
esp_err_t nvs_config_load_location(char *city, int8_t *tz_offset_hours);

/**
 * Save time format preference to NVS
 * @param use_24_hour true for 24-hour format, false for 12-hour format
 * @return ESP_OK on success
 */
esp_err_t nvs_config_save_time_format(bool use_24_hour);

/**
 * Load time format preference from NVS
 * @param use_24_hour Pointer to store time format preference
 * @return ESP_OK on success, ESP_ERR_NVS_NOT_FOUND if no preference stored (defaults to 24-hour)
 */
esp_err_t nvs_config_load_time_format(bool *use_24_hour);

/**
 * Clear all stored configuration data
 * @return ESP_OK on success
 */
esp_err_t nvs_config_clear_all(void);

#endif // NVS_CONFIG_H