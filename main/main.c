#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_config.h"
#include "wifi_prov.h"
#include "wifi_sta.h"
#include "ntp_sync.h"
#include "quote_reader.h"
#include "display_mgr.h"
#include "app_config.h"

static const char *TAG = "AUTHOR_CLOCK";

// ─── State machine ────────────────────────────────────────────────────────────
typedef enum {
    STATE_BOOT,
    STATE_PROVISION,
    STATE_CONNECT,
    STATE_SYNC_TIME,
    STATE_RUNNING,
    STATE_ERROR
} app_state_t;

static app_state_t s_state = STATE_BOOT;

// ─── Shared runtime data (protected by mutex) ─────────────────────────────────
static SemaphoreHandle_t s_mutex;
static quote_result_t s_quote;
static bool s_quote_valid = false;

// ─── Helpers ─────────────────────────────────────────────────────────────────
static const char *weekday_names[] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                                      "Thursday", "Friday", "Saturday"};
static const char *month_names[] = {"",        "January",  "February", "March",  "April",
                                    "May",     "June",     "July",     "August", "September",
                                    "October", "November", "December"};

static void build_date_string(char *buf, size_t len) {
    uint8_t hour, min, sec, day, month, weekday;
    uint16_t year;
    if (ntp_get_current_time(&hour, &min, &sec, &day, &month, &year, &weekday)) {
        snprintf(buf, len, "%s, %s %d", weekday_names[weekday % 7],
                 month_names[month <= 12 ? month : 0], day);
    } else {
        strlcpy(buf, "--", len);
    }
}

// ─── Provisioning callback ────────────────────────────────────────────────────
static void provisioning_done_cb(void) {
    ESP_LOGI(TAG, "Provisioning complete — restarting in 3 s");
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
}

// ─── FreeRTOS tasks ───────────────────────────────────────────────────────────
static void clock_task(void *arg) {
    uint8_t last_min = 0xFF;

    while (1) {
        uint8_t hour = 0, min = 0, sec = 0;
        bool time_ok = ntp_get_current_time(&hour, &min, &sec, NULL, NULL, NULL, NULL);

        if (time_ok && min != last_min) {
            last_min = min;
            ESP_LOGI(TAG, "Minute tick %02d:%02d — fetching quote", hour, min);

            quote_result_t q;
            esp_err_t ret = quote_read(hour, min, &q);

            xSemaphoreTake(s_mutex, portMAX_DELAY);
            if (ret == ESP_OK) {
                s_quote = q;
                s_quote_valid = true;
            } else {
                s_quote_valid = false;
                ESP_LOGW(TAG, "quote_read failed: %s", esp_err_to_name(ret));
            }
            xSemaphoreGive(s_mutex);

            char date_str[48];
            build_date_string(date_str, sizeof(date_str));

            xSemaphoreTake(s_mutex, portMAX_DELAY);
            quote_result_t q_copy = s_quote;
            bool q_valid = s_quote_valid;
            xSemaphoreGive(s_mutex);

            display_mgr_update(hour, min, date_str, q_valid ? &q_copy : NULL);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ─── app_main ─────────────────────────────────────────────────────────────────
void app_main(void) {
    ESP_LOGI(TAG, "=== Author Clock booting ===");

    // NVS first — required before everything else
    esp_err_t ret = nvs_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
        // Can't show error on display yet; just halt
        while (1)
            vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // On-device quote storage
    ret = quote_reader_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Quote reader init failed: %s — quotes will be unavailable",
                 esp_err_to_name(ret));
    }

    // Display init
    if (display_mgr_init() != 0) {
        ESP_LOGE(TAG, "Display init failed — continuing without display");
    }

    s_mutex = xSemaphoreCreateMutex();

    while (1) {
        switch (s_state) {

        // ── BOOT ──────────────────────────────────────────────────────────────
        case STATE_BOOT:
            if (nvs_config_has_wifi()) {
                ESP_LOGI(TAG, "WiFi credentials found → CONNECT");
                s_state = STATE_CONNECT;
            } else {
                ESP_LOGI(TAG, "No credentials → PROVISION");
                s_state = STATE_PROVISION;
            }
            break;

        // ── PROVISION ─────────────────────────────────────────────────────────
        case STATE_PROVISION:
            ESP_LOGI(TAG, "Starting provisioning AP");
            display_mgr_show_provisioning(WIFI_AP_SSID, WIFI_AP_IP);

            ret = wifi_prov_start(provisioning_done_cb);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "wifi_prov_start failed: %s", esp_err_to_name(ret));
                display_mgr_show_error("Provisioning failed. Restarting...");
                vTaskDelay(pdMS_TO_TICKS(3000));
                esp_restart();
            }
            // Block here; callback will restart device
            while (wifi_prov_is_active()) {
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
            break;

        // ── CONNECT ───────────────────────────────────────────────────────────
        case STATE_CONNECT: {
            char ssid[MAX_SSID_LEN + 1];
            char pass[MAX_PASS_LEN + 1];

            if (nvs_config_load_wifi(ssid, pass) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to load WiFi creds");
                s_state = STATE_PROVISION;
                break;
            }

            display_mgr_show_connecting(ssid);

            ret = author_wifi_sta_connect(ssid, pass);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "WiFi connected → SYNC_TIME");
                s_state = STATE_SYNC_TIME;
            } else {
                ESP_LOGE(TAG, "WiFi connect failed — clearing creds → PROVISION");
                display_mgr_show_error("Cannot connect to WiFi.\nReturning to setup...");
                nvs_config_clear_all();
                vTaskDelay(pdMS_TO_TICKS(3000));
                s_state = STATE_PROVISION;
            }
            break;
        }

        // ── SYNC_TIME ─────────────────────────────────────────────────────────
        case STATE_SYNC_TIME: {
            char city[MAX_CITY_LEN + 1];
            int8_t tz = 0;
            nvs_config_load_location(city, &tz);

            ntp_sync_start(tz);
            ESP_LOGI(TAG, "Waiting for NTP sync (UTC%+d)...", tz);
            bool synced = ntp_sync_wait(15000);
            if (!synced) {
                ESP_LOGW(TAG, "NTP timeout — continuing with unsynced time");
            }
            s_state = STATE_RUNNING;
            break;
        }

        // ── RUNNING ───────────────────────────────────────────────────────────
        case STATE_RUNNING: {
            ESP_LOGI(TAG, "Entering RUNNING state");

            // Show initial display before tasks start
            char date_str[48];
            build_date_string(date_str, sizeof(date_str));
            uint8_t h = 0, m = 0;
            ntp_get_current_time(&h, &m, NULL, NULL, NULL, NULL, NULL);
            display_mgr_update(h, m, date_str, NULL);

            // Start clock task — drives quote refresh every minute
            xTaskCreate(clock_task, "clock_task", CLOCK_TASK_STACK_SIZE, NULL, 5, NULL);

            // Block forever; clock_task drives everything
            while (1) {
                // Reconnect if WiFi drops
                if (!author_wifi_sta_is_connected()) {
                    ESP_LOGW(TAG, "WiFi lost — reconnecting...");
                    char ssid[MAX_SSID_LEN + 1];
                    char pass[MAX_PASS_LEN + 1];
                    if (nvs_config_load_wifi(ssid, pass) == ESP_OK) {
                        author_wifi_sta_connect(ssid, pass);
                    }
                }
                vTaskDelay(pdMS_TO_TICKS(30000));
            }
        }

        // ── ERROR ─────────────────────────────────────────────────────────────
        case STATE_ERROR:
            ESP_LOGE(TAG, "Fatal error state — restarting in 10 s");
            display_mgr_show_error("Fatal error. Restarting...");
            vTaskDelay(pdMS_TO_TICKS(10000));
            esp_restart();
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}