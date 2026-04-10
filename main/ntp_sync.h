#ifndef NTP_SYNC_H
#define NTP_SYNC_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "esp_err.h"

/**
 * @brief Start NTP synchronization
 *
 * Initializes SNTP (Simple Network Time Protocol) client and configures
 * the timezone offset. Must be called after WiFi connection is established.
 *
 * @param tz_offset_hours Timezone offset from UTC in hours (e.g., -8 for PST, +1 for CET)
 * @return esp_err_t ESP_OK on success, error code on failure
 */
esp_err_t ntp_sync_start(int8_t tz_offset_hours);

/**
 * @brief Wait for NTP synchronization to complete
 *
 * Polls the SNTP synchronization status until either sync is completed
 * or the timeout is reached.
 *
 * @param timeout_ms Maximum time to wait in milliseconds
 * @return true if sync completed successfully, false if timeout or error
 */
bool ntp_sync_wait(uint32_t timeout_ms);

/**
 * @brief Get current local time
 *
 * Retrieves the current time in local timezone (adjusted for the offset
 * set in ntp_sync_start). All parameters are output parameters and can
 * be NULL if not needed.
 *
 * @param hour Pointer to store hour (0-23), can be NULL
 * @param min Pointer to store minute (0-59), can be NULL
 * @param sec Pointer to store second (0-59), can be NULL
 * @param day Pointer to store day of month (1-31), can be NULL
 * @param month Pointer to store month (1-12), can be NULL
 * @param year Pointer to store year (e.g., 2024), can be NULL
 * @param weekday Pointer to store weekday (0=Sunday, 1=Monday, etc.), can be NULL
 * @return true if time is valid (synchronized), false if not synchronized yet
 */
bool ntp_get_current_time(uint8_t *hour, uint8_t *min, uint8_t *sec, uint8_t *day, uint8_t *month,
                          uint16_t *year, uint8_t *weekday);

/**
 * @brief Check if NTP time is synchronized
 *
 * @return true if time has been synchronized with NTP server, false otherwise
 */
bool ntp_is_synchronized(void);

/**
 * @brief Stop NTP synchronization
 *
 * Stops the SNTP service and cleans up resources.
 */
void ntp_sync_stop(void);

#endif // NTP_SYNC_H