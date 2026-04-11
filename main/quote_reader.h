#ifndef QUOTE_READER_H
#define QUOTE_READER_H

#include <stdint.h>
#include "esp_err.h"
#include "quote_types.h"

/**
 * @brief Initialize the on-device quote reader.
 *
 * Mounts the LittleFS "storage" partition at /quotes, opens quotes.bin,
 * validates the header, and caches the 1440-entry time index in RAM.
 * Must be called once at boot before quote_read().
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t quote_reader_init(void);

/**
 * @brief Read a random quote for the given time from on-device storage.
 *
 * Performs an O(1) index lookup then a sequential scan to the chosen record.
 * Does NOT require WiFi or network access.
 *
 * @param hour   0–23
 * @param minute 0–59
 * @param out    Caller-allocated struct to receive the result
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if no quotes exist for that time
 */
esp_err_t quote_read(uint8_t hour, uint8_t minute, quote_result_t *out);

/**
 * @brief Deinitialize and unmount the quote reader.
 *
 * Closes the open file handle and unmounts LittleFS.
 * Optional — only needed on orderly shutdown.
 */
void quote_reader_deinit(void);

#endif // QUOTE_READER_H
