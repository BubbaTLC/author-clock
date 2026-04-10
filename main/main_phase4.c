#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_config.h"
#include "wifi_prov.h"
#include "wifi_sta.h"

static const char *TAG = "MAIN_PHASE4";

// State machine states
typedef enum {
    STATE_BOOT,
    STATE_PROVISION,
    STATE_CONNECT,
    STATE_CONNECTED,
    STATE_ERROR
} app_state_t;

static app_state_t current_state = STATE_BOOT;

// Callback for when provisioning is complete
static void provisioning_complete_callback(void) {
    ESP_LOGI(TAG, "Provisioning complete! Restarting device in 3 seconds...");

    // Give time for HTTP response to be sent
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Restart the device
    esp_restart();
}

static void display_wifi_info(void) {
    char ssid[MAX_SSID_LEN + 1];
    char password[MAX_PASS_LEN + 1];
    char city[MAX_CITY_LEN + 1];
    int8_t timezone;

    if (nvs_config_load_wifi(ssid, password) == ESP_OK) {
        ESP_LOGI(TAG, "WiFi SSID: '%s'", ssid);
        ESP_LOGI(TAG, "Password length: %d characters", strlen(password));
    }

    if (nvs_config_load_location(city, &timezone) == ESP_OK) {
        ESP_LOGI(TAG, "City: '%s'", city);
        ESP_LOGI(TAG, "Timezone: UTC%+d", timezone);
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "=== ESP32 Author Clock - Phase 4 Test ===");
    ESP_LOGI(TAG, "Testing WiFi STA Connection Module");

    // Initialize NVS
    esp_err_t ret = nvs_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        current_state = STATE_ERROR;
        return;
    }

    // State machine main loop
    while (1) {
        switch (current_state) {
        case STATE_BOOT:
            ESP_LOGI(TAG, "=== BOOT STATE ===");
            if (nvs_config_has_wifi()) {
                ESP_LOGI(TAG, "WiFi credentials found, transitioning to CONNECT state");
                display_wifi_info();
                current_state = STATE_CONNECT;
            } else {
                ESP_LOGI(TAG, "No WiFi credentials found, transitioning to PROVISION state");
                current_state = STATE_PROVISION;
            }
            break;

        case STATE_PROVISION:
            ESP_LOGI(TAG, "=== PROVISION STATE ===");
            ESP_LOGI(TAG, "Starting WiFi provisioning...");

            ret = wifi_prov_start(provisioning_complete_callback);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to start WiFi provisioning: %s", esp_err_to_name(ret));
                current_state = STATE_ERROR;
                break;
            }

            ESP_LOGI(TAG, "=== SETUP INSTRUCTIONS ===");
            ESP_LOGI(TAG, "1. Connect your phone/computer to WiFi network: 'AuthorClock'");
            ESP_LOGI(TAG, "2. Open browser and go to: http://192.168.4.69");
            ESP_LOGI(TAG, "3. Fill in the configuration form");
            ESP_LOGI(TAG, "4. Submit the form to save settings");
            ESP_LOGI(TAG, "========================");

            // Keep provisioning running - callback will restart device
            while (wifi_prov_is_active()) {
                ESP_LOGI(TAG, "Provisioning mode active... waiting for configuration");
                vTaskDelay(pdMS_TO_TICKS(15000));
            }
            break;

        case STATE_CONNECT:
            ESP_LOGI(TAG, "=== CONNECT STATE ===");

            // Load WiFi credentials from NVS
            char ssid[MAX_SSID_LEN + 1];
            char password[MAX_PASS_LEN + 1];

            ret = nvs_config_load_wifi(ssid, password);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to load WiFi credentials: %s", esp_err_to_name(ret));
                current_state = STATE_ERROR;
                break;
            }

            ESP_LOGI(TAG, "Attempting to connect to WiFi: %s", ssid);

            // Attempt connection with timeout
            ret = wifi_sta_connect(ssid, password);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "Successfully connected to WiFi!");
                current_state = STATE_CONNECTED;
            } else {
                ESP_LOGE(TAG, "Failed to connect to WiFi after retries");
                ESP_LOGI(TAG, "Clearing WiFi config and returning to provisioning...");

                // Clear invalid credentials and go back to provisioning
                nvs_config_clear_all();
                current_state = STATE_PROVISION;
            }
            break;

        case STATE_CONNECTED:
            ESP_LOGI(TAG, "=== CONNECTED STATE ===");
            ESP_LOGI(TAG, "WiFi connection established successfully!");
            ESP_LOGI(TAG, "Connection status: %s",
                     wifi_sta_is_connected() ? "CONNECTED" : "DISCONNECTED");

            // Display stored configuration
            display_wifi_info();

            ESP_LOGI(TAG, "Phase 4 test complete! Device would now proceed to Phase 5 (NTP sync)");
            ESP_LOGI(TAG, "Staying connected for monitoring...");

            // Monitor connection every 30 seconds
            while (1) {
                if (!wifi_sta_is_connected()) {
                    ESP_LOGW(TAG, "WiFi connection lost! Attempting reconnection...");
                    current_state = STATE_CONNECT;
                    break;
                }

                ESP_LOGI(TAG, "WiFi still connected - monitoring...");
                vTaskDelay(pdMS_TO_TICKS(30000));
            }
            break;

        case STATE_ERROR:
            ESP_LOGE(TAG, "=== ERROR STATE ===");
            ESP_LOGE(TAG, "Application error occurred. Halting.");
            vTaskDelay(pdMS_TO_TICKS(10000));
            esp_restart();
            break;
        }

        // Small delay to prevent tight loop
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}