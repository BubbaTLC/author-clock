#include "nvs_config.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "NVS_CONFIG";
static const char *NVS_NAMESPACE = "clock_cfg";

// NVS keys
static const char *KEY_SSID = "ssid";
static const char *KEY_PASS = "pass";
static const char *KEY_CITY = "city";
static const char *KEY_TZ_OFF = "tz_off";
static const char *KEY_TIME_FMT = "time_fmt";

static nvs_handle_t nvs_handle_config = 0;
static bool nvs_initialized = false;

esp_err_t nvs_config_init(void) {
    esp_err_t ret;

    if (nvs_initialized) {
        ESP_LOGW(TAG, "NVS config already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing NVS flash");

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Open NVS namespace
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace: %s", esp_err_to_name(ret));
        return ret;
    }

    nvs_initialized = true;
    ESP_LOGI(TAG, "NVS config initialized successfully");
    return ESP_OK;
}

bool nvs_config_has_wifi(void) {
    if (!nvs_initialized) {
        ESP_LOGE(TAG, "NVS not initialized");
        return false;
    }

    size_t required_size = 0;
    esp_err_t ret = nvs_get_str(nvs_handle_config, KEY_SSID, NULL, &required_size);

    if (ret != ESP_OK || required_size <= 1) {
        // Key doesn't exist or is empty string
        ESP_LOGD(TAG, "No WiFi SSID stored or empty");
        return false;
    }

    ESP_LOGD(TAG, "WiFi SSID exists in NVS (size: %d)", required_size);
    return true;
}

esp_err_t nvs_config_save_wifi(const char *ssid, const char *password) {
    esp_err_t ret;

    if (!nvs_initialized) {
        ESP_LOGE(TAG, "NVS not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!ssid || !password) {
        ESP_LOGE(TAG, "Invalid parameters: ssid or password is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(ssid) == 0) {
        ESP_LOGE(TAG, "SSID cannot be empty");
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(ssid) > MAX_SSID_LEN || strlen(password) > MAX_PASS_LEN) {
        ESP_LOGE(TAG, "SSID or password too long");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Saving WiFi credentials: SSID='%s'", ssid);

    // Save SSID
    ret = nvs_set_str(nvs_handle_config, KEY_SSID, ssid);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save SSID: %s", esp_err_to_name(ret));
        return ret;
    }

    // Save password
    ret = nvs_set_str(nvs_handle_config, KEY_PASS, password);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save password: %s", esp_err_to_name(ret));
        return ret;
    }

    // Commit changes
    ret = nvs_commit(nvs_handle_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit WiFi credentials: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "WiFi credentials saved successfully");
    return ESP_OK;
}

esp_err_t nvs_config_load_wifi(char *ssid, char *password) {
    esp_err_t ret;
    size_t ssid_len = MAX_SSID_LEN + 1;
    size_t pass_len = MAX_PASS_LEN + 1;

    if (!nvs_initialized) {
        ESP_LOGE(TAG, "NVS not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!ssid || !password) {
        ESP_LOGE(TAG, "Invalid parameters: ssid or password buffer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Load SSID
    ret = nvs_get_str(nvs_handle_config, KEY_SSID, ssid, &ssid_len);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGD(TAG, "No WiFi SSID stored");
        } else {
            ESP_LOGE(TAG, "Failed to load SSID: %s", esp_err_to_name(ret));
        }
        return ret;
    }

    // Load password
    ret = nvs_get_str(nvs_handle_config, KEY_PASS, password, &pass_len);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGD(TAG, "No WiFi password stored");
        } else {
            ESP_LOGE(TAG, "Failed to load password: %s", esp_err_to_name(ret));
        }
        return ret;
    }

    ESP_LOGI(TAG, "WiFi credentials loaded: SSID='%s'", ssid);
    return ESP_OK;
}

esp_err_t nvs_config_save_location(const char *city, int8_t tz_offset_hours) {
    esp_err_t ret;

    if (!nvs_initialized) {
        ESP_LOGE(TAG, "NVS not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!city) {
        ESP_LOGE(TAG, "Invalid parameter: city is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(city) == 0 || strlen(city) > MAX_CITY_LEN) {
        ESP_LOGE(TAG, "City name invalid length");
        return ESP_ERR_INVALID_ARG;
    }

    if (tz_offset_hours < -12 || tz_offset_hours > 14) {
        ESP_LOGE(TAG, "Invalid timezone offset: %d (must be -12 to +14)", tz_offset_hours);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Saving location: city='%s', timezone=%+d", city, tz_offset_hours);

    // Save city
    ret = nvs_set_str(nvs_handle_config, KEY_CITY, city);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save city: %s", esp_err_to_name(ret));
        return ret;
    }

    // Save timezone offset
    ret = nvs_set_i8(nvs_handle_config, KEY_TZ_OFF, tz_offset_hours);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save timezone: %s", esp_err_to_name(ret));
        return ret;
    }

    // Commit changes
    ret = nvs_commit(nvs_handle_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit location data: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Location settings saved successfully");
    return ESP_OK;
}

esp_err_t nvs_config_load_location(char *city, int8_t *tz_offset_hours) {
    esp_err_t ret;
    size_t city_len = MAX_CITY_LEN + 1;

    if (!nvs_initialized) {
        ESP_LOGE(TAG, "NVS not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!city || !tz_offset_hours) {
        ESP_LOGE(TAG, "Invalid parameters: city or tz_offset_hours is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Load city
    ret = nvs_get_str(nvs_handle_config, KEY_CITY, city, &city_len);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGD(TAG, "No city stored");
        } else {
            ESP_LOGE(TAG, "Failed to load city: %s", esp_err_to_name(ret));
        }
        return ret;
    }

    // Load timezone offset
    ret = nvs_get_i8(nvs_handle_config, KEY_TZ_OFF, tz_offset_hours);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGD(TAG, "No timezone offset stored");
        } else {
            ESP_LOGE(TAG, "Failed to load timezone: %s", esp_err_to_name(ret));
        }
        return ret;
    }

    ESP_LOGI(TAG, "Location loaded: city='%s', timezone=%+d", city, *tz_offset_hours);
    return ESP_OK;
}

esp_err_t nvs_config_clear_all(void) {
    esp_err_t ret;

    if (!nvs_initialized) {
        ESP_LOGE(TAG, "NVS not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Clearing all stored configuration");

    // Erase all keys in our namespace
    ret = nvs_erase_all(nvs_handle_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear NVS data: %s", esp_err_to_name(ret));
        return ret;
    }

    // Commit changes
    ret = nvs_commit(nvs_handle_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit clear operation: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "All configuration data cleared");
    return ESP_OK;
}

esp_err_t nvs_config_save_time_format(bool use_24_hour) {
    esp_err_t ret;

    if (!nvs_initialized) {
        ESP_LOGE(TAG, "NVS not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Saving time format: %s", use_24_hour ? "24-hour" : "12-hour");

    // Save time format (1 = 24-hour, 0 = 12-hour)
    ret = nvs_set_u8(nvs_handle_config, KEY_TIME_FMT, use_24_hour ? 1 : 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save time format: %s", esp_err_to_name(ret));
        return ret;
    }

    // Commit changes
    ret = nvs_commit(nvs_handle_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit time format: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Time format saved successfully");
    return ESP_OK;
}

esp_err_t nvs_config_load_time_format(bool *use_24_hour) {
    esp_err_t ret;
    uint8_t format_val;

    if (!nvs_initialized) {
        ESP_LOGE(TAG, "NVS not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!use_24_hour) {
        ESP_LOGE(TAG, "Invalid parameter: use_24_hour is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Load time format
    ret = nvs_get_u8(nvs_handle_config, KEY_TIME_FMT, &format_val);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGD(TAG, "No time format stored, defaulting to 24-hour");
            *use_24_hour = true; // Default to 24-hour format
        } else {
            ESP_LOGE(TAG, "Failed to load time format: %s", esp_err_to_name(ret));
        }
        return ret;
    }

    *use_24_hour = (format_val != 0);
    ESP_LOGI(TAG, "Time format loaded: %s", *use_24_hour ? "24-hour" : "12-hour");
    return ESP_OK;
}