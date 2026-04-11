#ifndef WEATHER_FETCHER_H
#define WEATHER_FETCHER_H

#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Current weather data fetched from OpenWeatherMap.
 */
typedef struct {
    float temp_c;       /**< Temperature in Celsius */
    char condition[64]; /**< Short condition description, e.g. "Cloudy" */
    int weather_id;     /**< OWM weather condition code (for icon selection) */
    bool valid;         /**< false until first successful fetch */
} weather_data_t;

/**
 * @brief Fetch current weather for a city from OpenWeatherMap.
 *
 * Uses OWM_API_KEY (injected at compile time from .env) and the city name stored
 * in NVS. Requires active WiFi connection.
 *
 * @param city  City name string (e.g. "London")
 * @param out   Caller-allocated struct to receive the result
 * @return ESP_OK on success, error code on failure
 */
esp_err_t weather_fetch(const char *city, weather_data_t *out);

#endif // WEATHER_FETCHER_H
