#include "weather_fetcher.h"
#include "app_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"

static const char *TAG = "WEATHER";

// Response buffer for JSON parsing
#define WEATHER_BUF_SIZE 2048

typedef struct {
    char buf[WEATHER_BUF_SIZE];
    int len;
    bool error;
} weather_http_state_t;

static esp_err_t weather_http_event_handler(esp_http_client_event_t *evt) {
    weather_http_state_t *s = (weather_http_state_t *)evt->user_data;
    if (!s)
        return ESP_OK;

    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG, "HTTP error");
        s->error = true;
        break;
    case HTTP_EVENT_ON_DATA:
        if (evt->data_len > 0) {
            int remaining = WEATHER_BUF_SIZE - s->len - 1;
            if (remaining > 0) {
                int copy = evt->data_len < remaining ? evt->data_len : remaining;
                memcpy(s->buf + s->len, evt->data, copy);
                s->len += copy;
                s->buf[s->len] = '\0';
            }
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

// Extract a JSON number value for a given key (handles integer and float).
// Returns true and sets *out_str (null-terminated, up to out_size bytes) on success.
static bool json_extract_value(const char *json, const char *key, char *out_str, size_t out_size) {
    char needle[64];
    snprintf(needle, sizeof(needle), "\"%s\":", key);
    const char *p = strstr(json, needle);
    if (!p)
        return false;

    p += strlen(needle);
    while (*p == ' ')
        p++;

    size_t i = 0;
    while (*p && *p != ',' && *p != '}' && *p != '\n' && i < out_size - 1) {
        out_str[i++] = *p++;
    }
    out_str[i] = '\0';
    // Trim trailing spaces
    while (i > 0 && out_str[i - 1] == ' ')
        out_str[--i] = '\0';
    return (i > 0);
}

// Extract a JSON string value ("key":"value") — value without surrounding quotes.
static bool json_extract_string(const char *json, const char *key, char *out, size_t out_size) {
    char needle[64];
    snprintf(needle, sizeof(needle), "\"%s\":\"", key);
    const char *p = strstr(json, needle);
    if (!p) {
        snprintf(needle, sizeof(needle), "\"%s\": \"", key);
        p = strstr(json, needle);
    }
    if (!p)
        return false;

    p += strlen(needle);
    size_t i = 0;
    while (*p && *p != '"' && i < out_size - 1) {
        if (*p == '\\' && *(p + 1)) {
            p++;
            out[i++] = *p;
        } else {
            out[i++] = *p;
        }
        p++;
    }
    out[i] = '\0';
    return (i > 0);
}

esp_err_t weather_fetch(const char *city, weather_data_t *out) {
    if (!city || !out)
        return ESP_ERR_INVALID_ARG;

#ifndef OWM_API_KEY
    ESP_LOGE(TAG, "OWM_API_KEY not defined — rebuild with .env containing OPEN_WEATHER_MAP_KEY");
    return ESP_ERR_NOT_SUPPORTED;
#else
    char url[256];
    snprintf(url, sizeof(url), OWM_API_URL_FMT, city, OWM_API_KEY);

    weather_http_state_t *s = calloc(1, sizeof(weather_http_state_t));
    if (!s)
        return ESP_ERR_NO_MEM;

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = weather_http_event_handler,
        .user_data = s,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 15000,
        .buffer_size = 1024,
        .buffer_size_tx = 512,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        free(s);
        return ESP_FAIL;
    }

    esp_err_t ret = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(ret));
        free(s);
        return ret;
    }
    if (status != 200) {
        ESP_LOGE(TAG, "OWM returned HTTP %d for city '%s'", status, city);
        free(s);
        return ESP_FAIL;
    }
    if (s->error || s->len == 0) {
        free(s);
        return ESP_FAIL;
    }

    // Parse "temp" from "main" object
    char temp_str[32] = {0};
    const char *main_pos = strstr(s->buf, "\"main\":");
    if (main_pos && json_extract_value(main_pos, "temp", temp_str, sizeof(temp_str))) {
        out->temp_c = strtof(temp_str, NULL);
    } else {
        ESP_LOGW(TAG, "Could not parse temperature from response");
        free(s);
        return ESP_FAIL;
    }

    // Parse weather[0].id
    char id_str[16] = {0};
    const char *weather_pos = strstr(s->buf, "\"weather\":");
    if (weather_pos && json_extract_value(weather_pos, "id", id_str, sizeof(id_str))) {
        out->weather_id = atoi(id_str);
    } else {
        out->weather_id = 800; // default to clear
    }

    // Parse weather[0].icon (e.g. "01d", "10n")
    if (weather_pos) {
        json_extract_string(weather_pos, "icon", out->icon, sizeof(out->icon));
    }

    // Parse weather[0].description
    if (weather_pos) {
        json_extract_string(weather_pos, "description", out->condition, sizeof(out->condition));
        // Capitalize first letter
        if (out->condition[0] >= 'a' && out->condition[0] <= 'z') {
            out->condition[0] = out->condition[0] - 'a' + 'A';
        }
    }
    if (out->condition[0] == '\0') {
        strlcpy(out->condition, "Unknown", sizeof(out->condition));
    }

    out->valid = true;
    ESP_LOGI(TAG, "Weather fetched: %.1f°C, id=%d, icon=%s, '%s'", out->temp_c, out->weather_id,
             out->icon[0] ? out->icon : "(none)", out->condition);

    free(s);
    return ESP_OK;
#endif
}
