# Plan: ESP32 Author Clock + Weather Display

## Summary
Build an ESP32 e-paper author clock that displays time-matching literary quotes (fetched at runtime from the open-author-clock GitHub repo), weather info from OpenWeatherMap, and provides a secure WiFi captive-portal provisioning flow. Framework: ESP-IDF with CMake (existing project).

---

## Decisions
- **Framework**: ESP-IDF (existing CMake/sdkconfig setup)
- **Quotes**: Streamed from `https://raw.githubusercontent.com/ambercaravalho/open-author-clock/main/data.json` each minute — no local storage
- **Fonts**: Built-in Waveshare bitmap fonts (Font12, Font16, Font20, Font24)
- **Weather location**: User enters city name via captive portal web form
- **WiFi credential security**: NVS (no flash encryption; protected from casual access)
- **OWM API key**: Embedded at compile time via CMake `-D` flag (read from `.env`)
- **Display**: Waveshare 7.5" V2, 800×480, wired per SKILL.md pinout (SCK=13, MOSI=14, CS=15, RST=26, DC=27, BUSY=25)

---

## File Structure (target)
```
hello_world/
├── CMakeLists.txt                    (updated: set project name, remove MINIMAL_BUILD)
├── partitions.csv                    (custom: 2MB app, 16KB NVS, 16KB NVS keys)
├── main/
│   ├── CMakeLists.txt                (all srcs, REQUIRES list)
│   ├── main.c                        (boot state machine, FreeRTOS tasks)
│   ├── app_config.h                  (compile-time constants: OWM key, NTP server, GitHub URL)
│   ├── nvs_config.c / nvs_config.h   (NVS read/write for wifi creds, city, timezone)
│   ├── wifi_prov.c / wifi_prov.h     (AP mode + esp_http_server captive portal)
│   ├── wifi_sta.c / wifi_sta.h       (STA mode connect + event group)
│   ├── ntp_sync.c / ntp_sync.h       (esp_sntp init + wait-for-sync)
│   ├── quote_fetcher.c / quote_fetcher.h  (HTTPS stream + JSON scan + random pick)
│   ├── weather_fetcher.c / weather_fetcher.h (HTTPS GET + cJSON parse)
│   ├── display_mgr.c / display_mgr.h  (EPD layout + render)
│   └── epd/
│       ├── EPD_7in5_V2.c / .h
│       ├── GUI_Paint.c / .h
│       ├── DEV_Config.c / .h
│       └── fonts.c / .h
└── sdkconfig (rebuilt via menuconfig)
```

---

## Phase 1 — Project Scaffolding
1. Update root `CMakeLists.txt`:
   - Remove `idf_build_set_property(MINIMAL_BUILD ON)` 
   - Rename project to `author_clock`
   - Read OWM key from environment and pass as `-DOWM_API_KEY="..."` compile definition
2. Create `partitions.csv` with:
   - `nvs` 0x9000, size 0x6000 (24KB)
   - `phy_init` 0xF000, size 0x1000
   - `factory` 0x10000, size 0x300000 (3MB app)
3. Create `main/CMakeLists.txt` with all source files and `REQUIRES`:
   `esp_wifi esp_netif nvs_flash esp_event esp_http_server esp_http_client esp_tls cjson driver esp_timer lwip`
4. Create `main/app_config.h`:
   - `GITHUB_QUOTES_URL`: raw.githubusercontent.com URL
   - `NTP_SERVER`: "pool.ntp.org"
   - `OWM_API_URL_FMT`: format string for OpenWeatherMap endpoint
   - `OWM_API_KEY`: from compile-time define
   - `WIFI_AP_SSID`: "AuthorClock"
   - `WIFI_AP_IP`: "192.168.4.69"
   - `WEATHER_UPDATE_INTERVAL_S`: 1800 (30 min)
   - `CLOCK_UPDATE_INTERVAL_S`: 60 (1 min)
5. Copy Waveshare driver files into `main/epd/` (from .github/skills/waveshare/epd7in5_V2-demo/ reference)

---

## Phase 2 — NVS Config Module
`nvs_config.c/.h` — wraps NVS namespace "clock_cfg":
- `nvs_config_init()` — initialize NVS flash, handle ERR_NO_FREE_PAGES by erasing
- `nvs_config_has_wifi()` → bool — check if "ssid" key exists and non-empty
- `nvs_config_save_wifi(ssid, pass)` — write both strings to NVS, commit
- `nvs_config_load_wifi(ssid[33], pass[65])` — read both strings
- `nvs_config_save_location(city[64], tz_offset_hours)` — write city + int8 tz
- `nvs_config_load_location(city[64], *tz_offset_hours)` — read
- All keys: "ssid", "pass", "city", "tz_off"

---

## Phase 3 — WiFi Provisioning Module
`wifi_prov.c/.h`:
- `wifi_prov_start(on_complete_cb)` — init AP mode (SSID=AuthorClock, open), start `esp_http_server`
- Two HTTP handlers:
  - `GET /` → serve HTML form (inputs: WiFi SSID, WiFi Password, City, Timezone offset)
    - HTML: minimal responsive form, `method="POST" action="/configure"`
    - Shows `<h1>Author Clock Setup</h1>` and form fields
  - `POST /configure` → parse `application/x-www-form-urlencoded` body, validate inputs, call `nvs_config_save_wifi()` + `nvs_config_save_location()`, return HTML "Setup complete. Restarting..." then call `on_complete_cb`
- `wifi_prov_stop()` — stop HTTP server, stop AP WiFi
- Security: credentials travel only over local AP link (192.168.4.x), not internet-routed

---

## Phase 4 — WiFi STA Module
`wifi_sta.c/.h`:
- `wifi_sta_connect(ssid, pass)` → esp_err_t
  - Registers event handlers for WIFI_EVENT_STA_CONNECTED, IP_EVENT_STA_GOT_IP, WIFI_EVENT_STA_DISCONNECTED
  - xEventGroup with WIFI_CONNECTED_BIT / WIFI_FAIL_BIT
  - Retries up to MAX_RETRY (5), then sets FAIL bit
  - Returns ESP_OK when connected, ESP_FAIL on failure
- `wifi_sta_disconnect()`
- `wifi_sta_is_connected()` → bool

---

## Phase 5 — NTP Sync Module
`ntp_sync.c/.h`:
- `ntp_sync_start(tz_offset_hours)` — call `esp_sntp_init()`, set NTP server, set TZ string based on offset
- `ntp_sync_wait(timeout_ms)` → bool — poll `sntp_get_sync_status()` until SNTP_SYNC_STATUS_COMPLETED or timeout
- `ntp_get_current_time(hour, min, sec, day, month, year, weekday)` — wrapper around `localtime()`

---

## Phase 6 — Quote Fetcher Module
`quote_fetcher.c/.h`:

**Structs**:
```c
typedef struct {
    char quote[1024];
    char title[128];
    char author[128];
} quote_result_t;
```

**Algorithm** (streaming HTTPS scan):
- `quote_fetch(uint8_t hour, uint8_t min, quote_result_t *out)` → esp_err_t
  1. Format time string as `"HH:MM"` (zero-padded)
  2. Open HTTPS GET via `esp_http_client` with `esp_crt_bundle_attach` for TLS
  3. Process response in `HTTP_EVENT_ON_DATA` callback using a sliding 4KB window buffer
  4. In the buffer, search for `"time": "HH:MM"` pattern
  5. When match found: backtrack to find `{` that opened that object, accumulate bytes until matching `}`
  6. Parse the extracted JSON object with `cJSON_ParseWithLength`
  7. Store matching quotes in a local array (up to 32 entries)
  8. After request completes, pick a random index (using `esp_random()`) and copy into `out`
- `esp_http_client` used in non-blocking event mode with `esp_http_client_perform()`

---

## Phase 7 — Weather Fetcher Module
`weather_fetcher.c/.h`:

**Structs**:
```c
typedef struct {
    float temp_c;
    char condition[64];    // e.g. "Partly Cloudy"
    char icon_code[8];     // e.g. "02d"
    char city[64];
} weather_result_t;
```

- `weather_fetch(city, api_key, weather_result_t *out)` → esp_err_t
  1. Build URL: `https://api.openweathermap.org/data/2.5/weather?q={city}&appid={key}&units=metric`
  2. HTTPS GET via `esp_http_client`, read full response (typically <2KB)
  3. Parse with `cJSON_Parse`: extract `main.temp`, `weather[0].description`, `weather[0].icon`, `name`
  4. Fill `out` fields
- Weather is cached in a module-level struct. Refresh only when `weather_fetch()` is called (every 30 min from main loop)

---

## Phase 8 — Display Manager Module
`display_mgr.c/.h`:

**Layout** (800×480 pixels):
```
┌─────────────────────────────────────────────┬───────────────────────┐
│  TIME: 12:34  (Font24, top left, y=10)      │  WEATHER:             │
│  Date: Tuesday, April 9                     │  London  (Font12)     │
├─────────────────────────────────────────────│  22.1°C  (Font20)     │
│  "Quote text wrapped here, multiple lines   │  Partly Cloudy (F12)  │
│   as needed using Font16..."                │  [simple icon]        │
│                                             │                       │
│   — Title                                   │                       │
│     Author Name                             │                       │
└─────────────────────────────────────────────┴───────────────────────┘
  Divider: vertical line at x=600
  Weather box: x=605 to x=795, y=0 to y=200
  Time: x=10, y=10 (Font24)
  Date: x=10, y=50 (Font16)
  Horizontal rule at y=85
  Quote area: x=10, y=95 to y=430 (Font16, word-wrapped)
  Attribution: x=10, y=435 (Font12)
```

**Functions**:
- `display_mgr_init()` — `DEV_Module_Init()`, allocate BlackImage buffer, `EPD_7IN5_V2_Init()`, `EPD_7IN5_V2_Clear()`
- `display_mgr_show_provisioning(ap_ssid, ip_str)` — full refresh: "Connect to WiFi: AuthorClock / Visit: 192.168.4.69"
- `display_mgr_show_connecting(ssid)` — full refresh: "Connecting to {ssid}..."
- `display_mgr_show_error(msg)` — full refresh: error message
- `display_mgr_update(time_h, time_m, date_str, quote_result_t*, weather_result_t*)` — full EPD refresh:
  1. `Paint_Clear(WHITE)`
  2. Draw time string (Font24)
  3. Draw date string (Font16)
  4. Draw dividers
  5. Word-wrap and draw quote text (Font16) — `text_wrap()` helper function
  6. Draw attribution (Font12)
  7. Draw weather box (temp Font20, city + condition Font12)
  8. `display_draw_weather_icon(x, y, icon_code)` — map icon_code to simple geometric drawing:
     - "01d/01n": circle (sun/moon) via `Paint_DrawCircle`
     - "02*/03*/04*": overlapping circles (cloud)
     - "09*/10*": circles + downward lines (rain)
     - "11*": circles + zigzag (thunder)
     - "13*": asterisk pattern (snow)
     - "50*": horizontal lines (mist)
  9. `EPD_7IN5_V2_Display(BlackImage)` then `EPD_7IN5_V2_Sleep()`

**`text_wrap()` helper**: Takes string + font width/height + area width/height, breaks on word boundaries, calls `Paint_DrawString_EN` for each line, returns lines drawn.

---

## Phase 9 — Main State Machine
`main.c`:

**States**:
```c
typedef enum {
    STATE_BOOT,
    STATE_PROVISION,       // AP mode, show setup UI
    STATE_CONNECT,         // STA mode connecting
    STATE_SYNC_TIME,       // NTP sync
    STATE_RUNNING,         // Normal clock operation
    STATE_ERROR
} app_state_t;
```

**Boot flow**:
1. `nvs_config_init()`
2. `display_mgr_init()`
3. If `!nvs_config_has_wifi()` → STATE_PROVISION
4. STATE_PROVISION: load AP, show provisioning screen, wait for form submission callback
5. On callback: `esp_restart()`
6. Next boot → STATE_CONNECT
7. STATE_CONNECT: `wifi_sta_connect(ssid, pass)`, display "Connecting...", timeout 30s → STATE_PROVISION on fail
8. STATE_SYNC_TIME: `ntp_sync_start(tz)`, `ntp_sync_wait(10000ms)`, on fail → continue with unsynchronized time
9. STATE_RUNNING:
   - Fetch initial weather + quote
   - Create two FreeRTOS tasks:
     - **clock_task** (stack 8KB): wakes every second, on new minute triggers quote refresh + display update
     - **weather_task** (stack 4KB): sleeps 30 minutes, wakes + calls `weather_fetch()` to update cached result
   - Both tasks share a mutex protecting weather_result global

**Quote refresh in clock_task**:
- Get current HH:MM
- Call `quote_fetch(h, m, &quote_result)`
- Call `display_mgr_update(...)`

**Task priorities**: clock_task = 5, weather_task = 3

---

## Phase 10 — sdkconfig / menuconfig Edits
Required settings (in sdkconfig or via `sdkconfig.defaults`):
- `CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192`
- `CONFIG_PARTITION_TABLE_CUSTOM=y`
- `CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"`
- `CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y`
- `CONFIG_ESP_TLS_USE_SECURE_ELEMENT=n`
- `CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y` (for CA cert bundle for HTTPS)
- `CONFIG_MBEDTLS_CERTIFICATE_BUNDLE_DEFAULT_FULL=y`
- `CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM=10`
- `CONFIG_LWIP_LOCAL_HOSTNAME="authorclock"`
- FreeRTOS tickrate stays at default (100Hz is fine; timers use esp_timer for ms precision)

---

## Relevant Files to Create/Modify
- `CMakeLists.txt` — modify
- `partitions.csv` — create
- `sdkconfig` / `sdkconfig.defaults` — update
- `main/CMakeLists.txt` — create
- `main/main.c` — create
- `main/app_config.h` — create
- `main/nvs_config.c` / `main/nvs_config.h` — create
- `main/wifi_prov.c` / `main/wifi_prov.h` — create
- `main/wifi_sta.c` / `main/wifi_sta.h` — create
- `main/ntp_sync.c` / `main/ntp_sync.h` — create
- `main/quote_fetcher.c` / `main/quote_fetcher.h` — create
- `main/weather_fetcher.c` / `main/weather_fetcher.h` — create
- `main/display_mgr.c` / `main/display_mgr.h` — create
- `main/epd/EPD_7in5_V2.c` + `.h` — create (from Waveshare driver)
- `main/epd/GUI_Paint.c` + `.h` — create (from Waveshare driver)
- `main/epd/DEV_Config.c` + `.h` — create (from Waveshare driver)
- `main/epd/fonts.c` + `.h` — create (from Waveshare driver)

---

## Verification
1. Build: `idf.py build` — should compile without errors
2. Flash: `idf.py -p /dev/tty.usbserial-* flash monitor`
3. On first boot: ESP32 creates "AuthorClock" AP → navigate to 192.168.4.69 → form visible
4. Submit WiFi credentials + city → device restarts → connects to home network
5. After connection: NTP syncs, quote appears for current minute, weather in corner
6. Wait 1 minute → display refreshes with new quote for current time
7. Wait 30 minutes → weather data refreshes
8. Monitor serial for ESP_LOGI messages at each state transition
9. Verify credentials not visible in plaintext in NVS by checking NVS partition dump

---

## Excluded Scope
- HTTPS on the captive portal (HTTP only on local AP — acceptable security tradeoff)
- Multiple timezone support (single UTC offset integer)
- OTA firmware updates
- News/calendar features (extensibility noted in architecture)
- Display rotation modes (fixed ROTATE_0)
- 4-gray mode (binary black/white only)
