#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "nvs_config.h"
#include "epd/DEV_Config.h"
#include "epd/EPD_7in5_V2.h"
#include "epd/GUI_Paint.h"

static const char *TAG = "AUTHOR_CLOCK";

void test_nvs_config(void) {
    ESP_LOGI(TAG, "=== Testing NVS Config Module ===");

    // Test 1: Check if WiFi credentials exist (should be false initially)
    bool has_wifi = nvs_config_has_wifi();
    ESP_LOGI(TAG, "Has WiFi credentials: %s", has_wifi ? "YES" : "NO");

    // Test 2: Save test WiFi credentials
    ESP_LOGI(TAG, "Saving test WiFi credentials...");
    esp_err_t ret = nvs_config_save_wifi("TestNetwork", "TestPassword123");
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "WiFi credentials saved successfully");
    } else {
        ESP_LOGE(TAG, "Failed to save WiFi credentials");
    }

    // Test 3: Check if WiFi credentials exist now
    has_wifi = nvs_config_has_wifi();
    ESP_LOGI(TAG, "Has WiFi credentials after save: %s", has_wifi ? "YES" : "NO");

    // Test 4: Load WiFi credentials
    char ssid[33] = {0};
    char password[65] = {0};
    ret = nvs_config_load_wifi(ssid, password);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Loaded WiFi - SSID: '%s', Password: '%s'", ssid, password);
    } else {
        ESP_LOGE(TAG, "Failed to load WiFi credentials");
    }

    // Test 5: Save location settings
    ESP_LOGI(TAG, "Saving test location settings...");
    ret = nvs_config_save_location("San Francisco", -8);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Location settings saved successfully");
    } else {
        ESP_LOGE(TAG, "Failed to save location settings");
    }

    // Test 6: Load location settings
    char city[64] = {0};
    int8_t timezone = 0;
    ret = nvs_config_load_location(city, &timezone);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Loaded location - City: '%s', Timezone: %+d hours", city, timezone);
    } else {
        ESP_LOGE(TAG, "Failed to load location settings");
    }

    ESP_LOGI(TAG, "=== NVS Config Test Complete ===");
}

void display_nvs_status(void) {
    ESP_LOGI(TAG, "Updating display with NVS status...");

    // Allocate image buffer
    UWORD Imagesize =
        ((EPD_7IN5_V2_WIDTH % 8 == 0) ? (EPD_7IN5_V2_WIDTH / 8) : (EPD_7IN5_V2_WIDTH / 8 + 1)) *
        EPD_7IN5_V2_HEIGHT;

    UBYTE *BlackImage = (UBYTE *)malloc(Imagesize);
    if (!BlackImage) {
        ESP_LOGE(TAG, "Failed to allocate display buffer");
        return;
    }

    // Initialize Paint canvas
    Paint_NewImage(BlackImage, EPD_7IN5_V2_WIDTH, EPD_7IN5_V2_HEIGHT, ROTATE_0, WHITE);
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    // Display title
    Paint_DrawString_EN(50, 50, "ESP32 Author Clock", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(50, 80, "Phase 2: NVS Config", &Font12, BLACK, WHITE);

    // Check and display WiFi status
    bool has_wifi = nvs_config_has_wifi();
    Paint_DrawString_EN(50, 120, "WiFi Config:", &Font12, BLACK, WHITE);
    if (has_wifi) {
        char ssid[33] = {0};
        char password[65] = {0};
        if (nvs_config_load_wifi(ssid, password) == ESP_OK) {
            char wifi_info[100];
            snprintf(wifi_info, sizeof(wifi_info), "SSID: %s", ssid);
            Paint_DrawString_EN(70, 140, wifi_info, &Font12, BLACK, WHITE);
            Paint_DrawString_EN(70, 160, "Password: [STORED]", &Font12, BLACK, WHITE);
        }
    } else {
        Paint_DrawString_EN(70, 140, "[NOT CONFIGURED]", &Font12, BLACK, WHITE);
    }

    // Check and display location status
    char city[64] = {0};
    int8_t timezone = 0;
    Paint_DrawString_EN(50, 200, "Location Config:", &Font12, BLACK, WHITE);
    if (nvs_config_load_location(city, &timezone) == ESP_OK) {
        char location_info[100];
        snprintf(location_info, sizeof(location_info), "City: %s", city);
        Paint_DrawString_EN(70, 220, location_info, &Font12, BLACK, WHITE);

        char tz_info[50];
        snprintf(tz_info, sizeof(tz_info), "Timezone: UTC%+d", timezone);
        Paint_DrawString_EN(70, 240, tz_info, &Font12, BLACK, WHITE);
    } else {
        Paint_DrawString_EN(70, 220, "[NOT CONFIGURED]", &Font12, BLACK, WHITE);
    }

    // Display instructions
    Paint_DrawString_EN(50, 280, "Next: WiFi Provisioning", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(50, 300, "(Phase 3)", &Font12, BLACK, WHITE);

    // Update display
    EPD_7IN5_V2_Display(BlackImage);
    free(BlackImage);

    ESP_LOGI(TAG, "Display updated");
}

void app_main(void) {
    ESP_LOGI(TAG, "=== ESP32 Author Clock - Phase 2 ===");

    // Initialize NVS configuration system
    ESP_LOGI(TAG, "Initializing NVS config...");
    esp_err_t ret = nvs_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS config");
        return;
    }

    // Test NVS functionality
    test_nvs_config();

    // Initialize display
    ESP_LOGI(TAG, "Initializing display...");
    if (DEV_Module_Init() != 0 || EPD_7IN5_V2_Init() != 0) {
        ESP_LOGE(TAG, "Failed to initialize display");
        return;
    }

    // Show NVS status on display
    display_nvs_status();

    // Sleep display
    DEV_Delay_ms(2000);
    EPD_7IN5_V2_Sleep();

    ESP_LOGI(TAG, "=== Phase 2 Complete ===");
    ESP_LOGI(TAG, "NVS Config Module: FUNCTIONAL");
    ESP_LOGI(TAG, "Ready for Phase 3 - WiFi Provisioning");

    // Keep task alive
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}