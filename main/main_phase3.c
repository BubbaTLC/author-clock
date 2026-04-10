#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_config.h"
#include "wifi_prov.h"

static const char *TAG = "MAIN_PHASE3";

// Callback for when provisioning is complete
static void provisioning_complete_callback(void) {
    ESP_LOGI(TAG, "Provisioning complete! Restarting device in 3 seconds...");

    // Give time for HTTP response to be sent
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Restart the device
    esp_restart();
}

void app_main(void) {
    ESP_LOGI(TAG, "=== ESP32 Author Clock - Phase 3 Test ===");
    ESP_LOGI(TAG, "Testing WiFi Provisioning Module");

    // Initialize NVS
    esp_err_t ret = nvs_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return;
    }

    // Check if WiFi credentials already exist
    if (nvs_config_has_wifi()) {
        ESP_LOGI(TAG, "WiFi credentials found in NVS!");

        // Load and display stored credentials
        char ssid[MAX_SSID_LEN + 1];
        char password[MAX_PASS_LEN + 1];
        char city[MAX_CITY_LEN + 1];
        int8_t timezone;

        if (nvs_config_load_wifi(ssid, password) == ESP_OK) {
            ESP_LOGI(TAG, "Stored WiFi SSID: '%s'", ssid);
            ESP_LOGI(TAG, "Password length: %d characters", strlen(password));
        }

        if (nvs_config_load_location(city, &timezone) == ESP_OK) {
            ESP_LOGI(TAG, "Stored City: '%s'", city);
            ESP_LOGI(TAG, "Stored Timezone: UTC%+d", timezone);
        }

        ESP_LOGI(TAG, "Device already configured. To reconfigure, clear NVS storage.");
        ESP_LOGI(
            TAG,
            "In normal operation, device would now connect to WiFi and start main application.");

        // For testing, provide option to clear config
        ESP_LOGI(TAG, "Waiting 10 seconds... Press reset to test provisioning again");
        vTaskDelay(pdMS_TO_TICKS(10000));

        ESP_LOGI(TAG, "Clearing stored configuration for testing...");
        nvs_config_clear_all();
        ESP_LOGI(TAG, "Configuration cleared. Restarting...");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();

    } else {
        ESP_LOGI(TAG, "No WiFi credentials found. Starting provisioning mode...");

        // Start WiFi provisioning
        ret = wifi_prov_start(provisioning_complete_callback);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to start WiFi provisioning: %s", esp_err_to_name(ret));
            return;
        }

        ESP_LOGI(TAG, "WiFi provisioning started successfully!");
        ESP_LOGI(TAG, "=== SETUP INSTRUCTIONS ===");
        ESP_LOGI(TAG, "1. Connect your phone/computer to WiFi network: 'AuthorClock'");
        ESP_LOGI(TAG, "2. Open browser and go to: http://192.168.4.69");
        ESP_LOGI(TAG, "3. Fill in the configuration form");
        ESP_LOGI(TAG, "4. Submit the form to save settings");
        ESP_LOGI(TAG, "========================");

        // Keep the provisioning mode running
        while (wifi_prov_is_active()) {
            ESP_LOGI(TAG, "Provisioning mode active... waiting for configuration");
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }
}