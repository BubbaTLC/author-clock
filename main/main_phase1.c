#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "app_config.h"
#include "epd/DEV_Config.h"
#include "epd/EPD_7in5_V2.h"
#include "epd/GUI_Paint.h"

static const char *TAG = "AUTHOR_CLOCK";

void app_main(void) {
    ESP_LOGI(TAG, "=== ESP32 Author Clock Starting ===");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");

    // Test EPD initialization
    ESP_LOGI(TAG, "Initializing EPD hardware...");
    if (DEV_Module_Init() != 0) {
        ESP_LOGE(TAG, "EPD hardware initialization failed");
        return;
    }

    if (EPD_7IN5_V2_Init() != 0) {
        ESP_LOGE(TAG, "EPD display initialization failed");
        return;
    }

    ESP_LOGI(TAG, "EPD initialized successfully");

    // Allocate image buffer
    UWORD Imagesize =
        ((EPD_7IN5_V2_WIDTH % 8 == 0) ? (EPD_7IN5_V2_WIDTH / 8) : (EPD_7IN5_V2_WIDTH / 8 + 1)) *
        EPD_7IN5_V2_HEIGHT;

    UBYTE *BlackImage;
    if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        ESP_LOGE(TAG, "Failed to apply for black memory...");
        return;
    }
    ESP_LOGI(TAG, "Allocated %d bytes for display buffer", Imagesize);

    // Initialize Paint canvas
    Paint_NewImage(BlackImage, EPD_7IN5_V2_WIDTH, EPD_7IN5_V2_HEIGHT, ROTATE_0, WHITE);
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    ESP_LOGI(TAG, "Drawing basic test pattern...");

    // Draw some basic shapes first to test display functionality
    Paint_DrawRectangle(50, 50, 300, 100, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(60, 60, 290, 90, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    // Draw some lines
    Paint_DrawLine(50, 120, 300, 120, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    Paint_DrawLine(50, 130, 300, 160, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

    // Draw a circle
    Paint_DrawCircle(400, 100, 40, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(400, 100, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    // Test individual character drawing (since our font is limited)
    ESP_LOGI(TAG, "Testing character rendering...");
    // Draw space character (should be blank)
    Paint_DrawChar(50, 200, ' ', &Font12, BLACK, WHITE);
    // Draw exclamation mark (should show pattern)
    Paint_DrawChar(70, 200, '!', &Font12, BLACK, WHITE);

    // Try to draw simple text with available characters
    Paint_DrawString_EN(50, 250, "! ! !", &Font12, BLACK, WHITE);

    // Display the image
    ESP_LOGI(TAG, "Refreshing display...");
    EPD_7IN5_V2_Display(BlackImage);
    DEV_Delay_ms(2000);

    // Clean up and sleep
    ESP_LOGI(TAG, "Putting display to sleep");
    EPD_7IN5_V2_Sleep();
    free(BlackImage);

    ESP_LOGI(TAG, "=== Display Test Complete ===");

    // Keep the task alive
    while (1) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}