#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_config.h"
#include "wifi_sta.h"

static const char *TAG = "WIFI_STA_TEST";

void app_main(void) {
    ESP_LOGI(TAG, "=== WiFi STA Module Test ===");

    // Initialize NVS
    esp_err_t ret = nvs_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return;
    }

    // Test connection status
    bool connected = wifi_sta_is_connected();
    ESP_LOGI(TAG, "Initial WiFi connection status: %s", connected ? "CONNECTED" : "DISCONNECTED");

    // Test with dummy credentials (will fail but should not crash)
    ESP_LOGI(TAG, "Testing connection with dummy credentials...");
    ret = wifi_sta_connect("TestNetwork", "TestPassword");
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Connection successful (unexpected!)");
    } else {
        ESP_LOGI(TAG, "Connection failed as expected: %s", esp_err_to_name(ret));
    }

    // Test connection status again
    connected = wifi_sta_is_connected();
    ESP_LOGI(TAG, "Final WiFi connection status: %s", connected ? "CONNECTED" : "DISCONNECTED");

    ESP_LOGI(TAG, "WiFi STA module test completed successfully!");

    // Keep running
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}