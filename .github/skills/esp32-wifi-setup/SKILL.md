---
name: esp32-wifi-setup
description: Configure and implement WiFi connectivity in ESP32 projects using ESP-IDF
allowed-tools: fs_read, fs_write, execute_bash
---

# ESP32 WiFi Setup

## Instructions

1. **Add WiFi dependencies to CMakeLists.txt**
```cmake
idf_component_register(
    SRCS "main.c" "wifi_handler.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi nvs_flash esp_netif
)
```

2. **Initialize NVS (required for WiFi)**
```c
#include "nvs_flash.h"

esp_err_t ret = nvs_flash_init();
if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
}
ESP_ERROR_CHECK(ret);
```

3. **Create WiFi event handler**
```c
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define MAX_RETRY 5

static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
```

4. **Initialize WiFi in Station mode**
```c
esp_err_t wifi_init_sta(const char *ssid, const char *password)
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    return (bits & WIFI_CONNECTED_BIT) ? ESP_OK : ESP_FAIL;
}
```

5. **Use in app_main**
```c
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(wifi_init_sta("MyNetwork", "MyPassword"));
    // Your application code here
}
```

## Examples

### Basic Station Mode
```c
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(wifi_init_sta("MyNetwork", "MyPassword"));
    ESP_LOGI("app", "WiFi connected, starting application");
}
```

### With menuconfig
Add to `Kconfig.projbuild`:
```kconfig
menu "WiFi Configuration"
    config ESP_WIFI_SSID
        string "WiFi SSID"
        default "myssid"
    config ESP_WIFI_PASSWORD
        string "WiFi Password"
        default "mypassword"
endmenu
```

Use in code:
```c
#define WIFI_SSID CONFIG_ESP_WIFI_SSID
#define WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
```

## Troubleshooting

- **WiFi fails to initialize**: Ensure NVS is initialized first
- **Cannot connect to AP**: Verify SSID/password, check signal strength
- **IP not obtained**: Check DHCP server on router
- **Frequent disconnections**: Check power supply stability
