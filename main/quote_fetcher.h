#ifndef QUOTE_FETCHER_H
#define QUOTE_FETCHER_H

#include <stdint.h>
#include "esp_err.h"

typedef struct {
    char quote[1024];
    char title[128];
    char author[128];
    char timestring[128];
} quote_result_t;

/**
 * @brief Fetch a random quote matching the given time from GitHub
 *
 * Streams the remote JSON file over HTTPS, finds all entries matching
 * "HH:MM", picks one at random, and fills *out.
 *
 * @param hour  0-23
 * @param min   0-59
 * @param out   Caller-allocated struct to receive the result
 * @return ESP_OK on success, error code on failure
 */
esp_err_t quote_fetch(uint8_t hour, uint8_t min, quote_result_t *out);

#endif // QUOTE_FETCHER_H