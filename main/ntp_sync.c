#include "ntp_sync.h"
#include "app_config.h"

#include <string.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "esp_err.h"

static const char *TAG = "NTP_SYNC";

// Static variables
static bool s_ntp_initialized = false;
static bool s_time_synchronized = false;
static int8_t s_tz_offset_hours = 0;

// SNTP notification callback
static void time_sync_notification_cb(struct timeval *tv) {
    s_time_synchronized = true;
    ESP_LOGI(TAG, "Time synchronized from NTP server");

    // Log the synchronized time
    uint8_t hour, min, sec, day, month, weekday;
    uint16_t year;
    if (ntp_get_current_time(&hour, &min, &sec, &day, &month, &year, &weekday)) {
        ESP_LOGI(TAG, "Current time: %04d-%02d-%02d %02d:%02d:%02d (local time UTC%+d)", year,
                 month, day, hour, min, sec, s_tz_offset_hours);
    }
}

esp_err_t ntp_sync_start(int8_t tz_offset_hours) {
    ESP_LOGI(TAG, "Starting NTP synchronization with UTC%+d timezone", tz_offset_hours);

    // Timezone sanity check and helpful diagnostics
    if (tz_offset_hours > 0) {
        ESP_LOGW(TAG, "Timezone UTC%+d: Verify this is correct!", tz_offset_hours);
        ESP_LOGW(TAG, "Common zones: Saskatoon=UTC-6, Vancouver=UTC-8, Toronto=UTC-5, "
                      "London=UTC+0, Berlin=UTC+1");
        ESP_LOGW(TAG,
                 "If time shows wrong by 12h, timezone sign may be incorrect in stored config");
    }

    s_tz_offset_hours = tz_offset_hours;
    s_time_synchronized = false;

    // Configure timezone using POSIX format
    // POSIX TZ requires the offset in hours WEST of UTC (negative of standard UTC offset)
    // Example: For UTC-6 (like Saskatoon), local = UTC - 6, so POSIX TZ = "UTC+6"
    // Example: For UTC+2 (like Berlin), local = UTC + 2, so POSIX TZ = "UTC-2"
    char tz_string[32];
    snprintf(tz_string, sizeof(tz_string), "UTC%+d", -tz_offset_hours);

    ESP_LOGI(TAG, "Setting POSIX timezone to: %s (for local time UTC%+d)", tz_string,
             tz_offset_hours);
    setenv("TZ", tz_string, 1);
    tzset();

    // Initialize and configure SNTP
    if (!s_ntp_initialized) {
        ESP_LOGI(TAG, "Initializing SNTP client");
        esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, NTP_SERVER);
        esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
        esp_sntp_init();
        s_ntp_initialized = true;
        ESP_LOGI(TAG, "SNTP client initialized with server: %s", NTP_SERVER);
    }

    return ESP_OK;
}

bool ntp_sync_wait(uint32_t timeout_ms) {
    ESP_LOGI(TAG, "Waiting for NTP synchronization (timeout: %lu ms)", timeout_ms);

    if (!s_ntp_initialized) {
        ESP_LOGE(TAG, "NTP not initialized. Call ntp_sync_start() first");
        return false;
    }

    uint32_t start_time = xTaskGetTickCount();
    uint32_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    while (!s_time_synchronized) {
        // Check timeout
        if ((xTaskGetTickCount() - start_time) >= timeout_ticks) {
            ESP_LOGW(TAG, "NTP synchronization timeout after %lu ms", timeout_ms);
            return false;
        }

        // Check SNTP sync status
        sntp_sync_status_t sync_status = esp_sntp_get_sync_status();
        switch (sync_status) {
        case SNTP_SYNC_STATUS_RESET:
            ESP_LOGD(TAG, "SNTP sync status: RESET");
            break;
        case SNTP_SYNC_STATUS_COMPLETED:
            ESP_LOGI(TAG, "NTP synchronization completed successfully");
            s_time_synchronized = true;
            return true;
        case SNTP_SYNC_STATUS_IN_PROGRESS:
            ESP_LOGD(TAG, "SNTP sync in progress...");
            break;
        default:
            ESP_LOGW(TAG, "Unknown SNTP sync status: %d", sync_status);
            break;
        }

        // Wait a bit before checking again
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    return s_time_synchronized;
}

bool ntp_get_current_time(uint8_t *hour, uint8_t *min, uint8_t *sec, uint8_t *day, uint8_t *month,
                          uint16_t *year, uint8_t *weekday) {
    if (!s_time_synchronized) {
        ESP_LOGD(TAG, "Time not synchronized yet");
        return false;
    }

    time_t now;
    struct tm timeinfo;

    // Get current time
    time(&now);
    localtime_r(&now, &timeinfo);

    // Check if time is reasonable (after year 2020)
    if (timeinfo.tm_year < (2020 - 1900)) {
        ESP_LOGW(TAG, "Time seems invalid: year %d", timeinfo.tm_year + 1900);
        return false;
    }

    // Fill output parameters if provided
    if (hour)
        *hour = timeinfo.tm_hour;
    if (min)
        *min = timeinfo.tm_min;
    if (sec)
        *sec = timeinfo.tm_sec;
    if (day)
        *day = timeinfo.tm_mday;
    if (month)
        *month = timeinfo.tm_mon + 1; // tm_mon is 0-11, convert to 1-12
    if (year)
        *year = timeinfo.tm_year + 1900; // tm_year is years since 1900
    if (weekday)
        *weekday = timeinfo.tm_wday; // 0=Sunday, 1=Monday, etc.

    return true;
}

bool ntp_is_synchronized(void) {
    return s_time_synchronized;
}

void ntp_sync_stop(void) {
    if (s_ntp_initialized) {
        ESP_LOGI(TAG, "Stopping SNTP client");
        esp_sntp_stop();
        s_ntp_initialized = false;
        s_time_synchronized = false;
        ESP_LOGI(TAG, "SNTP client stopped");
    }
}