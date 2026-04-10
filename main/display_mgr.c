#include "display_mgr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"

#include "EPD_7in5_V2.h"
#include "GUI_Paint.h"
#include "fonts.h"

static const char *TAG = "DISPLAY_MGR";

// ─── Layout constants ────────────────────────────────────────────────────────
#define EPD_W 800
#define EPD_H 480

#define DIVIDER_X 600 // vertical divider between clock and (future) weather
#define TIME_X 10
#define TIME_Y 10
#define DATE_X 10
#define DATE_Y 50
#define HRULE_Y 85
#define QUOTE_X 10
#define QUOTE_Y 95
#define QUOTE_MAX_Y 420
#define ATTRIB_Y 432

// Right panel (weather placeholder / future use)
#define PANEL_X 610
#define PANEL_Y_START 10

// ─── Image buffer ─────────────────────────────────────────────────────────────
// 1 bit per pixel → (800 * 480) / 8 = 48000 bytes
#define IMAGE_SIZE (EPD_W * EPD_H / 8)
static uint8_t *s_image = NULL;
static bool s_initialized = false;

// ─── Helper: word-wrap text into the EPD ─────────────────────────────────────
// Returns the number of lines drawn.
static int text_wrap(const char *text, sFONT *font, uint16_t x, uint16_t y, uint16_t max_width,
                     uint16_t max_y) {
    if (!text || !font)
        return 0;

    int char_w = font->Width;
    int char_h = font->Height;
    int cols = max_width / char_w;
    int lines = 0;
    uint16_t cur_y = y;

    char line_buf[128];
    const char *p = text;

    while (*p && cur_y + char_h <= max_y) {
        // Find how many characters fit on this line (word-boundary break)
        int len = 0;
        int last_space = -1;

        while (p[len] && len < cols) {
            if (p[len] == ' ')
                last_space = len;
            if (p[len] == '\n') {
                last_space = len;
                break;
            }
            len++;
        }

        // If the remaining text fits entirely, take it all
        if (p[len] == '\0') {
            // fall through with len as-is
        } else if (last_space > 0) {
            len = last_space; // break at last word boundary
        }
        // else no space found — hard-break at col width

        // Copy and draw
        int copy = len < (int)sizeof(line_buf) - 1 ? len : (int)sizeof(line_buf) - 1;
        memcpy(line_buf, p, copy);
        line_buf[copy] = '\0';

        Paint_DrawString_EN(x, cur_y, line_buf, font, BLACK, WHITE);

        // Advance past the line content and optional space/newline
        p += len;
        if (*p == ' ' || *p == '\n')
            p++;

        cur_y += char_h + 2;
        lines++;
    }
    return lines;
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

    s_image = malloc(IMAGE_SIZE);
    if (!s_image) {
        ESP_LOGE(TAG, "Failed to allocate image buffer (%d bytes)", IMAGE_SIZE);
        return -2;
    }

    Paint_NewImage(s_image, EPD_W, EPD_H, ROTATE_0, WHITE);
    Paint_Clear(WHITE);

    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Clear();

    s_initialized = true;
    ESP_LOGI(TAG, "Display initialized (800x480, %d byte buffer)", IMAGE_SIZE);
    return 0;
}

void display_mgr_show_provisioning(const char *ap_ssid, const char *ip_str) {
    if (!s_initialized)
        return;
    ESP_LOGI(TAG, "Showing provisioning screen");

    Paint_Clear(WHITE);

    Paint_DrawString_EN(10, 80, "Author Clock Setup", &Font24, BLACK, WHITE);
    Paint_DrawString_EN(10, 130, "1. Connect your phone to WiFi:", &Font16, BLACK, WHITE);

    char ssid_line[64];
    snprintf(ssid_line, sizeof(ssid_line), "   \"%s\"", ap_ssid ? ap_ssid : "AuthorClock");
    Paint_DrawString_EN(10, 155, ssid_line, &Font20, BLACK, WHITE);

    Paint_DrawString_EN(10, 210, "2. Open a browser and visit:", &Font16, BLACK, WHITE);

    char url_line[64];
    snprintf(url_line, sizeof(url_line), "   http://%s", ip_str ? ip_str : "192.168.4.69");
    Paint_DrawString_EN(10, 235, url_line, &Font20, BLACK, WHITE);

    Paint_DrawString_EN(10, 295, "3. Fill in the form with your", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(10, 318, "   WiFi credentials and city.", &Font16, BLACK, WHITE);

    flush_display();
}

void display_mgr_show_connecting(const char *ssid) {
    if (!s_initialized)
        return;
    ESP_LOGI(TAG, "Showing connecting screen for: %s", ssid ? ssid : "?");

    Paint_Clear(WHITE);
    Paint_DrawString_EN(10, 200, "Connecting to WiFi...", &Font24, BLACK, WHITE);

    if (ssid) {
        char buf[64];
        snprintf(buf, sizeof(buf), "\"%s\"", ssid);
        Paint_DrawString_EN(10, 240, buf, &Font20, BLACK, WHITE);
    }

    flush_display();
}

void display_mgr_show_error(const char *msg) {
    if (!s_initialized)
        return;
    ESP_LOGE(TAG, "Showing error: %s", msg ? msg : "(null)");

    Paint_Clear(WHITE);
    Paint_DrawString_EN(10, 190, "Error:", &Font24, BLACK, WHITE);

    if (msg) {
        text_wrap(msg, &Font16, 10, 230, EPD_W - 20, EPD_H - 20);
    }

    flush_display();
}

void display_mgr_update(uint8_t time_h, uint8_t time_m, const char *date_str,
                        const quote_result_t *quote) {
    if (!s_initialized)
        return;
    ESP_LOGI(TAG, "Updating display: %02d:%02d", time_h, time_m);

    Paint_Clear(WHITE);

    // ── Debug: filled black rectangle — confirms EPD pipeline is working ──────
    Paint_DrawRectangle(620, 5, 795, 55, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(625, 15, "DEBUG OK", &Font12, WHITE, BLACK);
    Paint_DrawString_EN(625, 33, "white on blk", &Font12, WHITE, BLACK);

    // ── Time ──────────────────────────────────────────────────────────────────
    char time_str[8];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", time_h, time_m);
    Paint_DrawString_EN(TIME_X, TIME_Y, time_str, &Font24, BLACK, WHITE);

    // ── Date ──────────────────────────────────────────────────────────────────
    if (date_str) {
        Paint_DrawString_EN(DATE_X, DATE_Y, date_str, &Font16, BLACK, WHITE);
    }

    // ── Horizontal rule below date ────────────────────────────────────────────
    Paint_DrawLine(0, HRULE_Y, DIVIDER_X, HRULE_Y, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    // ── Vertical divider ──────────────────────────────────────────────────────
    Paint_DrawLine(DIVIDER_X, 0, DIVIDER_X, EPD_H, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    // Right panel intentionally left blank (weather module not implemented)

    // ── Quote ─────────────────────────────────────────────────────────────────
    if (quote && quote->quote[0]) {
        text_wrap(quote->quote, &Font16, QUOTE_X, QUOTE_Y, DIVIDER_X - QUOTE_X - 4, ATTRIB_Y - 4);

        // Attribution: — Title / Author
        char attrib[288];
        snprintf(attrib, sizeof(attrib), "\x97 %s / %s", quote->title, quote->author);
        text_wrap(attrib, &Font12, QUOTE_X, ATTRIB_Y, DIVIDER_X - QUOTE_X - 4, EPD_H - 2);
    } else {
        Paint_DrawString_EN(QUOTE_X, QUOTE_Y + 60, "Fetching quote...", &Font16, BLACK, WHITE);
    }

    flush_display();
    ESP_LOGI(TAG, "Display update complete");
}