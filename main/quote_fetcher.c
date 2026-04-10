#include "quote_fetcher.h"
#include "app_config.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_random.h"

static const char *TAG = "QUOTE_FETCH";

// Maximum number of matching quotes we collect before picking one
#define MAX_MATCHES 32
// Sliding window buffer size for scanning the HTTP response stream
#define WINDOW_SIZE 4096

// ─── Internal state shared across HTTP event callbacks ───────────────────────

typedef struct {
    char time_pattern[24]; // e.g. "\"time\": \"12:34\""

    // Sliding window over raw response bytes
    char window[WINDOW_SIZE + 1];
    int window_len;

    // When we have found a time-pattern hit we accumulate a JSON object here
    bool in_object;     // currently capturing an object
    int brace_depth;    // nested { } tracker
    char obj_buf[2048]; // buffer for one JSON object
    int obj_len;

    // Collected matching quotes
    quote_result_t matches[MAX_MATCHES];
    int match_count;

    // Sticky error flag
    bool error;
} fetch_state_t;

// ─── Helper: extract a JSON string field from a raw object snippet ───────────
// Looks for: "key": "value" and copies the unescaped value into dest[dest_size].
// Returns true on success.
static bool json_extract_string(const char *obj, const char *key, char *dest, size_t dest_size) {
    // Build needle: "key": "
    char needle[64];
    snprintf(needle, sizeof(needle), "\"%s\": \"", key);
    const char *p = strstr(obj, needle);
    if (!p) {
        // try without space: "key":"
        snprintf(needle, sizeof(needle), "\"%s\":\"", key);
        p = strstr(obj, needle);
    }
    if (!p)
        return false;

    p += strlen(needle); // now points at first char of value

    size_t out = 0;
    while (*p && out < dest_size - 1) {
        if (*p == '\\' && *(p + 1)) {
            p++;
            switch (*p) {
            case '"':
                dest[out++] = '"';
                break;
            case '\\':
                dest[out++] = '\\';
                break;
            case 'n':
                dest[out++] = '\n';
                break;
            case 't':
                dest[out++] = '\t';
                break;
            default:
                dest[out++] = *p;
                break;
            }
        } else if (*p == '"') {
            break; // end of value
        } else {
            dest[out++] = *p;
        }
        p++;
    }
    dest[out] = '\0';
    return (out > 0);
}

// ─── Helper: flush the object buffer and parse it ────────────────────────────

static void try_parse_object(fetch_state_t *s) {
    if (s->obj_len <= 0)
        return;
    s->obj_buf[s->obj_len] = '\0';

    char quote[1024], title[128], author[128];
    if (json_extract_string(s->obj_buf, "quote", quote, sizeof(quote)) &&
        json_extract_string(s->obj_buf, "title", title, sizeof(title)) &&
        json_extract_string(s->obj_buf, "author", author, sizeof(author)) &&
        s->match_count < MAX_MATCHES) {
        quote_result_t *q = &s->matches[s->match_count];
        strlcpy(q->quote, quote, sizeof(q->quote));
        strlcpy(q->title, title, sizeof(q->title));
        strlcpy(q->author, author, sizeof(q->author));
        s->match_count++;
        ESP_LOGD(TAG, "Matched quote #%d: \"%s\" — %s", s->match_count, q->title, q->author);
    }
}

// ─── Helper: process a chunk of data into the state machine ──────────────────

static void process_chunk(fetch_state_t *s, const char *data, int len) {
    for (int i = 0; i < len; i++) {
        char c = data[i];

        if (!s->in_object) {
            // Maintain sliding window
            if (s->window_len < WINDOW_SIZE) {
                s->window[s->window_len++] = c;
            } else {
                // Shift window left by 1
                memmove(s->window, s->window + 1, WINDOW_SIZE - 1);
                s->window[WINDOW_SIZE - 1] = c;
                s->window_len = WINDOW_SIZE;
            }
            s->window[s->window_len] = '\0';

            // Check if window ends with the time pattern
            int pat_len = strlen(s->time_pattern);
            if (s->window_len >= pat_len &&
                memcmp(s->window + s->window_len - pat_len, s->time_pattern, pat_len) == 0) {
                // Backtrack in the window to find the opening '{' of this object
                int start = s->window_len - pat_len;
                while (start > 0 && s->window[start] != '{')
                    start--;

                // Start accumulating the object from '{'
                int copy_len = s->window_len - start;
                if (copy_len >= (int)sizeof(s->obj_buf))
                    copy_len = sizeof(s->obj_buf) - 1;
                memcpy(s->obj_buf, s->window + start, copy_len);
                s->obj_len = copy_len;

                // Count braces already in the buffer
                s->brace_depth = 0;
                for (int k = 0; k < s->obj_len; k++) {
                    if (s->obj_buf[k] == '{')
                        s->brace_depth++;
                    else if (s->obj_buf[k] == '}')
                        s->brace_depth--;
                }

                s->in_object = (s->brace_depth > 0);

                if (!s->in_object && s->brace_depth == 0 && s->obj_len > 2) {
                    // Already have a complete object
                    try_parse_object(s);
                    s->obj_len = 0;
                }

                // Reset window so we don't double-match
                s->window_len = 0;
            }
        } else {
            // Accumulate into obj_buf
            if (s->obj_len < (int)sizeof(s->obj_buf) - 1) {
                s->obj_buf[s->obj_len++] = c;
            }
            if (c == '{')
                s->brace_depth++;
            else if (c == '}') {
                s->brace_depth--;
                if (s->brace_depth == 0) {
                    s->in_object = false;
                    try_parse_object(s);
                    s->obj_len = 0;
                }
            }
        }
    }
}

// ─── HTTP event handler ───────────────────────────────────────────────────────

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    fetch_state_t *s = (fetch_state_t *)evt->user_data;
    if (!s)
        return ESP_OK;

    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG, "HTTP error event");
        s->error = true;
        break;

    case HTTP_EVENT_ON_DATA:
        if (evt->data_len > 0) {
            process_chunk(s, (const char *)evt->data, evt->data_len);
        }
        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP transfer finished, matched %d quote(s)", s->match_count);
        break;

    default:
        break;
    }
    return ESP_OK;
}

// ─── Public API ──────────────────────────────────────────────────────────────

esp_err_t quote_fetch(uint8_t hour, uint8_t min, quote_result_t *out) {
    if (!out)
        return ESP_ERR_INVALID_ARG;

    // Allocate state on heap — too large for stack
    fetch_state_t *s = calloc(1, sizeof(fetch_state_t));
    if (!s) {
        ESP_LOGE(TAG, "Out of memory allocating fetch state");
        return ESP_ERR_NO_MEM;
    }

    // Build the search pattern: "time": "HH:MM"
    snprintf(s->time_pattern, sizeof(s->time_pattern), "\"time\": \"%02u:%02u\"", hour, min);
    ESP_LOGI(TAG, "Fetching quotes for time pattern: %s", s->time_pattern);

    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = GITHUB_QUOTES_URL,
        .event_handler = http_event_handler,
        .user_data = s,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 30000,
        .buffer_size = 2048,
        .buffer_size_tx = 512,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to init HTTP client");
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
        ESP_LOGE(TAG, "HTTP returned status %d", status);
        free(s);
        return ESP_FAIL;
    }
    if (s->error || s->match_count == 0) {
        ESP_LOGW(TAG, "No quotes found for %02u:%02u", hour, min);
        free(s);
        return ESP_ERR_NOT_FOUND;
    }

    // Pick a random match
    int idx = (int)(esp_random() % (uint32_t)s->match_count);
    *out = s->matches[idx];
    ESP_LOGI(TAG, "Selected quote %d/%d: \"%s\" — %s, \"%s\"", idx + 1, s->match_count, out->title,
             out->author, out->quote);

    free(s);
    return ESP_OK;
}