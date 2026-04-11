#include "display_mgr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_config.h"

#include "EPD_7in5_V2.h"
#include "GUI_Paint.h"
#include "bmfont_renderer.h"
#include "fonts/font_select.h"

static const char *TAG = "DISPLAY_MGR";

// ─── Layout constants ────────────────────────────────────────────────────────
#define EPD_W 800
#define EPD_H 480

#define TIME_X 10
#define TIME_Y 10
#define DATE_X 10
#define DATE_Y 50
#define HRULE_Y 45
#define QUOTE_X 20          // Reduce left margin to give more space
#define QUOTE_Y 80          // Start higher to leave room for larger text
#define QUOTE_MAX_WIDTH 760 // Use more of the screen width
#define QUOTE_MAX_Y 360     // Max Y for quote text (leaves room for attribution below)

// ─── Image buffer ─────────────────────────────────────────────────────────────
// 1 bit per pixel → (800 * 480) / 8 = 48000 bytes
#define IMAGE_SIZE (EPD_W * EPD_H / 8)
static uint8_t *s_image = NULL;
static bool s_initialized = false;

// ─── Quote persistence ────────────────────────────────────────────────────────

// ─── Helper: enhanced word-wrap with bmfont support ──────────────────────────
// Returns the Y position immediately after the last drawn line (useful for placing
// elements dynamically below the text block). Handles timeString portion in bold.
// ─── Helper: measure line width using the correct font per segment ─────────────
// Uses bold_font for characters that fall within [ts_pos, ts_pos+ts_len),
// regular font for everything else. Matches how the line will actually be drawn.
static uint16_t mixed_string_width(const char *p, int len, const bmfont_t *font,
                                   const bmfont_t *bold_font, const char *ts_pos, int ts_len) {
    char buf[512];
    int n;

    // Fast path: no bold overlap in this segment
    if (!bold_font || !ts_pos || ts_len == 0 || ts_pos >= p + len || ts_pos + ts_len <= p) {
        n = len < (int)sizeof(buf) - 1 ? len : (int)sizeof(buf) - 1;
        memcpy(buf, p, n);
        buf[n] = '\0';
        return bmfont_string_width(buf, font);
    }

    uint16_t w = 0;

    // Prefix in regular font: [p, ts_pos)
    if (ts_pos > p) {
        n = (int)(ts_pos - p);
        n = n < (int)sizeof(buf) - 1 ? n : (int)sizeof(buf) - 1;
        memcpy(buf, p, n);
        buf[n] = '\0';
        w += bmfont_string_width(buf, font);
    }

    // Bold section: intersect [ts_pos, ts_pos+ts_len) with [p, p+len)
    const char *bold_start = ts_pos > p ? ts_pos : p;
    const char *bold_end = (ts_pos + ts_len) < (p + len) ? (ts_pos + ts_len) : (p + len);
    if (bold_end > bold_start) {
        n = (int)(bold_end - bold_start);
        n = n < (int)sizeof(buf) - 1 ? n : (int)sizeof(buf) - 1;
        memcpy(buf, bold_start, n);
        buf[n] = '\0';
        w += bmfont_string_width(buf, bold_font);
    }

    // Suffix in regular font: [ts_pos+ts_len, p+len)
    if (ts_pos + ts_len < p + len) {
        const char *sfx = ts_pos + ts_len;
        n = (int)((p + len) - sfx);
        n = n < (int)sizeof(buf) - 1 ? n : (int)sizeof(buf) - 1;
        memcpy(buf, sfx, n);
        buf[n] = '\0';
        w += bmfont_string_width(buf, font);
    }

    return w;
}

static int text_wrap_bmfont(const char *text, const char *timestring, const bmfont_t *font,
                            const bmfont_t *bold_font, uint16_t x, uint16_t y, uint16_t max_width,
                            uint16_t max_y) {
    if (!text || !font)
        return 0;

    uint16_t line_height = bmfont_height(font);
    uint16_t bold_line_height = bold_font ? bmfont_height(bold_font) : line_height;
    if (bold_line_height > line_height)
        line_height = bold_line_height;

    int lines = 0;
    uint16_t cur_y = y;

    char line_buf[128];
    const char *p = text;
    const char *timestring_pos = timestring ? strstr(text, timestring) : NULL;
    int ts_len = (timestring && timestring_pos) ? (int)strlen(timestring) : 0;

    while (*p && cur_y + line_height <= max_y) {
        // Find how many characters fit on this line (word-boundary break)
        int len = 0;
        int last_space = -1;
        uint16_t line_width = 0;

        // Calculate how much text fits, measuring bold sections with bold_font
        while (p[len]) {
            if (p[len] == ' ')
                last_space = len;
            if (p[len] == '\n') {
                last_space = len;
                break;
            }

            line_width = mixed_string_width(p, len + 1, font, bold_font, timestring_pos, ts_len);

            if (line_width >= max_width) {
                break;
            }

            len++;
        }

        // If the remaining text fits entirely, take it all
        if (p[len] == '\0') {
            // fall through with len as-is
        } else if (last_space > 0) {
            len = last_space; // break at last word boundary
        } else if (len == 0) {
            len = 1; // Force at least one character to avoid infinite loop
        }

        // Copy line
        int copy = len < (int)sizeof(line_buf) - 1 ? len : (int)sizeof(line_buf) - 1;
        memcpy(line_buf, p, copy);
        line_buf[copy] = '\0';

        // Check if this line contains the timestring
        bool has_timestring = timestring_pos && (timestring_pos >= p) && (timestring_pos < p + len);

        if (has_timestring && bold_font && timestring) {
            // Split line at timestring for special formatting
            int prefix_len = timestring_pos - p;
            int timestring_len = strlen(timestring);

            uint16_t cur_x = x;

            // Draw text before timestring
            if (prefix_len > 0) {
                char prefix[128];
                int prefix_copy = prefix_len < 127 ? prefix_len : 127;
                memcpy(prefix, p, prefix_copy);
                prefix[prefix_copy] = '\0';
                cur_x += bmfont_draw_string(cur_x, cur_y, prefix, font, BLACK, WHITE);
            }

            // Draw timestring in bold font
            cur_x += bmfont_draw_string(cur_x, cur_y, timestring, bold_font, BLACK, WHITE);

            // Draw text after timestring
            const char *suffix = timestring_pos + timestring_len;
            if (suffix < p + len) {
                int suffix_len = (p + len) - suffix;
                if (suffix_len > 0 && suffix_len < 128) {
                    char suffix_buf[128];
                    memcpy(suffix_buf, suffix, suffix_len);
                    suffix_buf[suffix_len] = '\0';
                    bmfont_draw_string(cur_x, cur_y, suffix_buf, font, BLACK, WHITE);
                }
            }
        } else {
            // Regular text drawing
            bmfont_draw_string(x, cur_y, line_buf, font, BLACK, WHITE);
        }

        // Advance past the line content and optional space/newline
        p += len;
        if (*p == ' ' || *p == '\n')
            p++;

        cur_y += line_height + 8; // Line spacing for better readability
        lines++;
    }
    return cur_y;
}

// ─── Helper: clean text encoding issues ───────────────────────────────────────
static void clean_text_encoding(char *text) {
    if (!text)
        return;

    char *p = text;
    while (*p) {
        // Replace UTF-8 smart quotes with regular ASCII quotes
        if ((unsigned char)*p == 0xe2 && (unsigned char)*(p + 1) == 0x80) {
            if ((unsigned char)*(p + 2) == 0x98 || (unsigned char)*(p + 2) == 0x99) {
                // Left/right single quotes → regular apostrophe
                *p = '\'';
                memmove(p + 1, p + 3, strlen(p + 3) + 1);
            } else if ((unsigned char)*(p + 2) == 0x9c || (unsigned char)*(p + 2) == 0x9d) {
                // Left/right double quotes → regular quotes
                *p = '"';
                memmove(p + 1, p + 3, strlen(p + 3) + 1);
            } else if ((unsigned char)*(p + 2) == 0x94) {
                // Em dash → regular dash
                *p = '-';
                memmove(p + 1, p + 3, strlen(p + 3) + 1);
            } else {
                p++;
            }
        } else if ((unsigned char)*p >= 128) {
            // Replace any other non-ASCII characters with space
            *p = ' ';
            p++;
        } else {
            p++;
        }
    }
}

// ─── Helper: format combined date/time string ────────────────────────────────
static void format_datetime_string(char *buf, size_t len, uint8_t hour, uint8_t min,
                                   const char *date_str, bool use_24_hour) {
    if (!buf || !date_str)
        return;

    if (use_24_hour) {
        snprintf(buf, len, "%s, %02d:%02d", date_str, hour, min);
    } else {
        // Convert to 12-hour format
        const char *period = "AM";
        uint8_t display_hour = hour;

        if (hour == 0) {
            display_hour = 12; // midnight
        } else if (hour == 12) {
            period = "PM"; // noon
        } else if (hour > 12) {
            display_hour = hour - 12;
            period = "PM";
        }

        snprintf(buf, len, "%s, %d:%02d %s", date_str, display_hour, min, period);
    }
}

// ─── Helper: commit buffer to display ────────────────────────────────────────
// Always re-init before displaying: the display enters deep sleep after each
// EPD_7IN5_V2_Sleep() call and must be woken with a full init sequence before
// it will respond to commands or toggle the BUSY pin again.
static void flush_display(void) {
    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Display(s_image);
    EPD_7IN5_V2_Sleep();
}

// ─── Public API ──────────────────────────────────────────────────────────────

int display_mgr_init(void) {
    if (s_initialized)
        return 0;

    ESP_LOGI(TAG, "Initializing e-paper display");

    if (DEV_Module_Init() != 0) {
        ESP_LOGE(TAG, "DEV_Module_Init failed");
        return -1;
    }

    // Give the display hardware time to stabilize after power-on
    vTaskDelay(pdMS_TO_TICKS(2000));

    s_image = malloc(IMAGE_SIZE);
    if (!s_image) {
        ESP_LOGE(TAG, "Failed to allocate image buffer (%d bytes)", IMAGE_SIZE);
        return -2;
    }

    Paint_NewImage(s_image, EPD_W, EPD_H, ROTATE_0, WHITE);
    Paint_Clear(WHITE);

    // First initialization after cold boot - may need retry
    ESP_LOGI(TAG, "Performing initial EPD setup (may take extra time on cold boot)");
    for (int attempt = 0; attempt < 2; attempt++) {
        if (attempt > 0) {
            ESP_LOGW(TAG, "Retrying EPD initialization (attempt %d)", attempt + 1);
            vTaskDelay(pdMS_TO_TICKS(3000)); // Extra delay between attempts
        }

        EPD_7IN5_V2_Init();
        // Don't do initial clear on cold boot - it often fails and isn't necessary
        // since we'll be updating with content immediately anyway
        break;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "Display initialized (800x480, %d byte buffer)", IMAGE_SIZE);
    return 0;
}

void display_mgr_show_provisioning(const char *ap_ssid, const char *ip_str) {
    if (!s_initialized)
        return;
    ESP_LOGI(TAG, "Showing provisioning screen");

    Paint_Clear(WHITE);

    bmfont_draw_string(10, 80, "Author Clock Setup", &font_ui, BLACK, WHITE);
    bmfont_draw_string(10, 130, "1. Connect your phone to WiFi:", &font_ui, BLACK, WHITE);

    char ssid_line[64];
    snprintf(ssid_line, sizeof(ssid_line), "   \"%s\"", ap_ssid ? ap_ssid : "AuthorClock");
    bmfont_draw_string(10, 155, ssid_line, &font_ui, BLACK, WHITE);

    bmfont_draw_string(10, 210, "2. Open a browser and visit:", &font_ui, BLACK, WHITE);

    char url_line[64];
    snprintf(url_line, sizeof(url_line), "   http://%s", ip_str ? ip_str : "192.168.4.69");
    bmfont_draw_string(10, 235, url_line, &font_ui, BLACK, WHITE);

    bmfont_draw_string(10, 295, "3. Fill in the form with your", &font_ui, BLACK, WHITE);
    bmfont_draw_string(10, 318, "   WiFi credentials and city.", &font_ui, BLACK, WHITE);

    flush_display();
}

void display_mgr_show_connecting(const char *ssid) {
    if (!s_initialized)
        return;
    ESP_LOGI(TAG, "Showing connecting screen for: %s", ssid ? ssid : "?");

    Paint_Clear(WHITE);
    bmfont_draw_string(10, 200, "Connecting to WiFi...", &font_ui, BLACK, WHITE);

    if (ssid) {
        char buf[64];
        snprintf(buf, sizeof(buf), "\"%s\"", ssid);
        bmfont_draw_string(10, 240, buf, &font_ui, BLACK, WHITE);
    }

    flush_display();
}

void display_mgr_show_error(const char *msg) {
    if (!s_initialized)
        return;
    ESP_LOGE(TAG, "Showing error: %s", msg ? msg : "(null)");

    Paint_Clear(WHITE);
    bmfont_draw_string(10, 190, "Error:", &font_ui, BLACK, WHITE);

    if (msg) {
        text_wrap_bmfont(msg, NULL, &font_ui, NULL, 10, 230, EPD_W - 20, EPD_H - 20);
    }

    flush_display();
}

void display_mgr_update(uint8_t time_h, uint8_t time_m, const char *date_str,
                        const quote_result_t *quote) {
    if (!s_initialized)
        return;
    ESP_LOGI(TAG, "Updating display: %02d:%02d", time_h, time_m);

    Paint_Clear(WHITE);

    // Load time format preference from NVS
    bool use_24_hour = true;                   // default
    nvs_config_load_time_format(&use_24_hour); // ignore errors, use default

    // ── Combined Date/Time ────────────────────────────────────────────────────
    char datetime_str[80];
    format_datetime_string(datetime_str, sizeof(datetime_str), time_h, time_m, date_str,
                           use_24_hour);
    bmfont_draw_string(TIME_X, TIME_Y, datetime_str, &font_ui, BLACK, WHITE);

    // ── Horizontal rule below date ────────────────────────────────────────────
    Paint_DrawLine(0, HRULE_Y, EPD_W, HRULE_Y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    // ── Quote ─────────────────────────────────────────────────────────────────
    if (quote && quote->quote[0]) {
        // Add quotation marks around quote text
        char quoted_text[1026]; // quote[1024] + 2 quotes + null terminator
        snprintf(quoted_text, sizeof(quoted_text), "\"%s\"", quote->quote);

        // Clean up any encoding issues in the quote text
        clean_text_encoding(quoted_text);

        const char *timestring = (quote->timestring[0]) ? quote->timestring : NULL;

        int quote_end_y = text_wrap_bmfont(quoted_text, timestring, &font_body, &font_body_bold,
                                           QUOTE_X, QUOTE_Y, QUOTE_MAX_WIDTH, QUOTE_MAX_Y);

        // Attribution: Title - Author, always 30px below the last line of the quote
        char attrib[288];
        snprintf(attrib, sizeof(attrib), "%s - %s", quote->title, quote->author);
        bmfont_draw_string(QUOTE_X, quote_end_y + 30, attrib, &font_attrib, BLACK, WHITE);
    } else {
        // Show message with current time. Format the time part separately so it
        // can be passed as the timestring and rendered in bold.
        char time_str[16];
        char no_quote_msg[96];

        if (use_24_hour) {
            snprintf(time_str, sizeof(time_str), "%02d:%02d", time_h, time_m);
        } else {
            uint8_t display_hour = time_h;
            const char *ampm = "AM";
            if (time_h == 0) {
                display_hour = 12;
            } else if (time_h == 12) {
                ampm = "PM";
            } else if (time_h > 12) {
                display_hour = time_h - 12;
                ampm = "PM";
            }
            snprintf(time_str, sizeof(time_str), "%d:%02d %s", display_hour, time_m, ampm);
        }

        snprintf(no_quote_msg, sizeof(no_quote_msg),
                 "Oh no looks like this time doesn't have a quote, but it is %s.", time_str);

        text_wrap_bmfont(no_quote_msg, time_str, &font_body, &font_body_bold, QUOTE_X, QUOTE_Y,
                         QUOTE_MAX_WIDTH, QUOTE_MAX_Y);
    }

    flush_display();
    ESP_LOGI(TAG, "Display update complete");
}