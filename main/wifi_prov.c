#include "wifi_prov.h"
#include "nvs_config.h"
#include "app_config.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "mdns.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "WIFI_PROV";

// HTTP server handle
static httpd_handle_t server = NULL;
static wifi_prov_complete_cb_t complete_callback = NULL;
static bool provisioning_active = false;
static esp_netif_t *ap_netif = NULL;

// DNS server for captive portal
static int dns_socket = -1;
static TaskHandle_t dns_task_handle = NULL;

// Enhanced HTML interface with separate forms and WiFi status
static const char *enhanced_config_html_template =
    "<!DOCTYPE html>"
    "<html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Author Clock Configuration</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;max-width:500px;margin:20px "
    "auto;padding:20px;background:#f5f5f5}"
    ".container{background:white;padding:30px;border-radius:8px;box-shadow:0 2px 10px "
    "rgba(0,0,0,0.1);margin-bottom:20px}"
    "h1{color:#333;text-align:center;margin-bottom:30px}"
    "h2{color:#666;border-bottom:2px solid #4CAF50;padding-bottom:10px}"
    ".status{background:#e8f4f8;padding:15px;border-radius:4px;margin-bottom:20px;font-size:14px}"
    ".form-group{margin-bottom:15px}"
    "label{display:block;margin-bottom:5px;color:#555;font-weight:bold}"
    "input,select{width:100%%;padding:10px;border:1px solid "
    "#ddd;border-radius:4px;font-size:16px;box-sizing:border-box}"
    "input:focus,select:focus{outline:none;border-color:#4CAF50}"
    ".btn{width:100%%;padding:12px;border:none;border-radius:4px;font-size:16px;cursor:pointer;"
    "margin-bottom:10px}"
    ".btn-primary{background:#4CAF50;color:white}.btn-primary:hover{background:#45a049}"
    ".btn-danger{background:#f44336;color:white}.btn-danger:hover{background:#da190b}"
    ".current-value{font-size:12px;color:#666;margin-bottom:5px}"
    "</style>"
    "</head><body>"
    "<div class='container'>"
    "<h1>📚 Author Clock Configuration</h1>"
    "<div class='status'>"
    "<strong>Current Status:</strong><br>"
    "WiFi: Connected to %s (Signal: %d dBm)<br>"
    "Location: %s<br>"
    "Timezone: UTC%+d<br>"
    "Time Format: %s<br>"
    "Access URL: <a href='http://authorclock.local'>authorclock.local</a>"
    "</div>"
    "</div>"

    "<div class='container'>"
    "<h2>📍 Location & Time Settings</h2>"
    "<form method='POST' action='/location'>"
    "<div class='form-group'>"
    "<div class='current-value'>Current: %s</div>"
    "<label for='city'>City for Weather:</label>"
    "<input type='text' id='city' name='city' maxlength='63' placeholder='London, New York, "
    "Tokyo...' value='%s'>"
    "</div>"
    "<div class='form-group'>"
    "<div class='current-value'>Current: UTC%+d</div>"
    "<label for='timezone'>Timezone Offset (hours from UTC):</label>"
    "<select id='timezone' name='timezone'>"
    "%s" // Timezone options will be inserted here
    "</select>"
    "</div>"
    "<div class='form-group'>"
    "<div class='current-value'>Current: %s</div>"
    "<label for='timeformat'>Time Format:</label>"
    "<select id='timeformat' name='timeformat'>"
    "%s" // Time format options will be inserted here
    "</select>"
    "</div>"
    "<button type='submit' class='btn btn-primary'>Update Location & Time</button>"
    "</form>"
    "</div>"

    "<div class='container'>"
    "<h2>📶 WiFi Settings</h2>"
    "<form method='POST' action='/wifi'>"
    "<div class='form-group'>"
    "<label for='ssid'>WiFi Network Name (SSID):</label>"
    "<input type='text' id='ssid' name='ssid' required maxlength='32' placeholder='Your WiFi "
    "network name'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='password'>WiFi Password:</label>"
    "<input type='password' id='password' name='password' required maxlength='64' "
    "placeholder='Your WiFi password'>"
    "</div>"
    "<button type='submit' class='btn btn-primary'>Update WiFi Settings</button>"
    "</form>"
    "</div>"

    "<div class='container'>"
    "<h2>🔧 Advanced Options</h2>"
    "<form method='POST' action='/factory-reset' onsubmit='return confirm(\"Are you sure? This "
    "will erase all settings and restart the device.\")'>"
    "<button type='submit' class='btn btn-danger'>Factory Reset</button>"
    "</form>"
    "<p style='font-size:12px;color:#666;text-align:center;margin-top:20px;'>This will erase all "
    "WiFi credentials and location settings.</p>"
    "</div>"
    "</body></html>";

// Generate timezone options HTML with current selection
static const char *generate_timezone_options(int8_t current_tz) {
    static char options[2048];
    options[0] = '\0';

    for (int tz = -12; tz <= 14; tz++) {
        char option[128];
        snprintf(option, sizeof(option), "<option value='%d'%s>UTC%+d</option>", tz,
                 (tz == current_tz) ? " selected" : "", tz);
        strcat(options, option);
    }

    return options;
}

// Generate time format options HTML with current selection
static const char *generate_timeformat_options(bool current_24h) {
    static char options[256];
    snprintf(options, sizeof(options),
             "<option value='1'%s>24-hour (15:30)</option>"
             "<option value='0'%s>12-hour (3:30 PM)</option>",
             current_24h ? " selected" : "", current_24h ? "" : " selected");
    return options;
}

static const char *success_html =
    "<!DOCTYPE html>"
    "<html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Setup Complete</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;max-width:400px;margin:50px "
    "auto;padding:20px;background:#f5f5f5}"
    ".container{background:white;padding:30px;border-radius:8px;box-shadow:0 2px 10px "
    "rgba(0,0,0,0.1);text-align:center}"
    "h1{color:#4CAF50;margin-bottom:20px}"
    ".success{background:#d4edda;color:#155724;padding:15px;border-radius:4px;margin-bottom:20px}"
    "</style>"
    "</head><body>"
    "<div class='container'>"
    "<h1>✅ Setup Complete!</h1>"
    "<div class='success'>"
    "Your Author Clock has been configured successfully. The device will restart and connect to "
    "your WiFi network."
    "</div>"
    "<p>You can now close this browser tab.</p>"
    "</div>"
    "</body></html>";

// URL decode function
static int url_decode(char *dest, const char *src, size_t dest_size) {
    size_t src_len = strlen(src);
    size_t dest_idx = 0;

    for (size_t i = 0; i < src_len && dest_idx < dest_size - 1; i++) {
        if (src[i] == '%' && i + 2 < src_len) {
            // URL decode %XX
            int hex_val;
            if (sscanf(src + i + 1, "%2x", &hex_val) == 1) {
                dest[dest_idx++] = (char)hex_val;
                i += 2; // Skip the hex digits
            } else {
                dest[dest_idx++] = src[i];
            }
        } else if (src[i] == '+') {
            // Replace + with space
            dest[dest_idx++] = ' ';
        } else {
            dest[dest_idx++] = src[i];
        }
    }
    dest[dest_idx] = '\0';
    return dest_idx;
}

// Parse form data
static bool parse_form_data(const char *data, char *ssid, char *password, char *city,
                            int8_t *timezone) {
    char *data_copy = strdup(data);
    if (!data_copy) {
        ESP_LOGE(TAG, "Failed to allocate memory for form data");
        return false;
    }

    bool ssid_found = false, pass_found = false, city_found = false, tz_found = false;

    char *token = strtok(data_copy, "&");
    while (token != NULL) {
        char *equals = strchr(token, '=');
        if (equals) {
            *equals = '\0';
            char *key = token;
            char *value = equals + 1;

            if (strcmp(key, "ssid") == 0) {
                url_decode(ssid, value, MAX_SSID_LEN + 1);
                ssid_found = true;
            } else if (strcmp(key, "password") == 0) {
                url_decode(password, value, MAX_PASS_LEN + 1);
                pass_found = true;
            } else if (strcmp(key, "city") == 0) {
                url_decode(city, value, MAX_CITY_LEN + 1);
                city_found = true;
            } else if (strcmp(key, "timezone") == 0) {
                int tz_int = atoi(value);
                if (tz_int >= -12 && tz_int <= 14) {
                    *timezone = (int8_t)tz_int;
                    tz_found = true;
                }
            }
        }
        token = strtok(NULL, "&");
    }

    free(data_copy);
    return ssid_found && pass_found && city_found && tz_found;
}

// Parse location form data (city and timezone only)
static bool parse_location_form_data(const char *data, char *city, int8_t *timezone,
                                     bool *use_24_hour) {
    if (!data || !city || !timezone || !use_24_hour)
        return false;

    char *data_copy = strdup(data);
    if (!data_copy)
        return false;

    bool city_found = false, tz_found = false, tf_found = false;

    char *token = strtok(data_copy, "&");
    while (token != NULL) {
        char *equals = strchr(token, '=');
        if (equals) {
            *equals = '\0';
            char *key = token;
            char *value = equals + 1;

            if (strcmp(key, "city") == 0) {
                url_decode(city, value, MAX_CITY_LEN + 1);
                city_found = true;
            } else if (strcmp(key, "timezone") == 0) {
                int tz_int = atoi(value);
                if (tz_int >= -12 && tz_int <= 14) {
                    *timezone = (int8_t)tz_int;
                    tz_found = true;
                }
            } else if (strcmp(key, "timeformat") == 0) {
                int tf_int = atoi(value);
                *use_24_hour = (tf_int != 0);
                tf_found = true;
            }
        }
        token = strtok(NULL, "&");
    }

    free(data_copy);
    return city_found && tz_found && tf_found;
}

// Parse WiFi form data (SSID and password only)
static bool parse_wifi_form_data(const char *data, char *ssid, char *password) {
    if (!data || !ssid || !password)
        return false;

    char *data_copy = strdup(data);
    if (!data_copy)
        return false;

    bool ssid_found = false, pass_found = false;

    char *token = strtok(data_copy, "&");
    while (token != NULL) {
        char *equals = strchr(token, '=');
        if (equals) {
            *equals = '\0';
            char *key = token;
            char *value = equals + 1;

            if (strcmp(key, "ssid") == 0) {
                url_decode(ssid, value, MAX_SSID_LEN + 1);
                ssid_found = true;
            } else if (strcmp(key, "password") == 0) {
                url_decode(password, value, MAX_PASS_LEN + 1);
                pass_found = true;
            }
        }
        token = strtok(NULL, "&");
    }

    free(data_copy);
    return ssid_found && pass_found;
}

// Captive portal detection handler - redirects to our setup page
static esp_err_t captive_portal_handler(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Location", "http://" WIFI_AP_IP "/");
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// DNS server task - responds to all queries with our AP IP
static void dns_server_task(void *pvParameters) {
    char rx_buffer[DNS_MAX_LEN];
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_UDP;
    struct sockaddr_in dest_addr;

    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(DNS_PORT);

    dns_socket = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (dns_socket < 0) {
        ESP_LOGE(TAG, "Unable to create DNS socket: errno %d", errno);
        goto cleanup;
    }
    ESP_LOGI(TAG, "DNS socket created");

    int err = bind(dns_socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        goto cleanup;
    }
    ESP_LOGI(TAG, "DNS socket bound, port %d", DNS_PORT);

    while (provisioning_active) {
        struct sockaddr_storage source_addr;
        socklen_t socklen = sizeof(source_addr);
        int len = recvfrom(dns_socket, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&source_addr, &socklen);

        if (len < 0) {
            ESP_LOGE(TAG, "DNS recvfrom failed: errno %d", errno);
            break;
        }

        // Get the sender's ip address as string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str,
                        sizeof(addr_str) - 1);
        }

        // Basic DNS response - redirect everything to our AP IP (192.168.4.69)
        if (len >= 12) { // Minimum DNS query size
            // Copy query ID
            rx_buffer[2] = 0x81;         // Response + recursion available
            rx_buffer[3] = 0x80;         // No error
            rx_buffer[6] = rx_buffer[4]; // Copy question count to answer count
            rx_buffer[7] = rx_buffer[5];
            rx_buffer[8] = 0; // Authority RRs
            rx_buffer[9] = 0;
            rx_buffer[10] = 0; // Additional RRs
            rx_buffer[11] = 0;

            // Append answer (A record pointing to 192.168.4.69)
            int answer_len = len;
            // Pointer to question name
            rx_buffer[answer_len++] = 0xc0;
            rx_buffer[answer_len++] = 0x0c;
            // Type A
            rx_buffer[answer_len++] = 0x00;
            rx_buffer[answer_len++] = 0x01;
            // Class IN
            rx_buffer[answer_len++] = 0x00;
            rx_buffer[answer_len++] = 0x01;
            // TTL (1 second)
            rx_buffer[answer_len++] = 0x00;
            rx_buffer[answer_len++] = 0x00;
            rx_buffer[answer_len++] = 0x00;
            rx_buffer[answer_len++] = 0x01;
            // Data length
            rx_buffer[answer_len++] = 0x00;
            rx_buffer[answer_len++] = 0x04;
            // IP address 192.168.4.69
            rx_buffer[answer_len++] = 192;
            rx_buffer[answer_len++] = 168;
            rx_buffer[answer_len++] = 4;
            rx_buffer[answer_len++] = 69;

            int err = sendto(dns_socket, rx_buffer, answer_len, 0, (struct sockaddr *)&source_addr,
                             sizeof(source_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during DNS sending: errno %d", errno);
            }
        }
    }

cleanup:
    if (dns_socket != -1) {
        ESP_LOGE(TAG, "Shutting down DNS socket");
        shutdown(dns_socket, 0);
        close(dns_socket);
        dns_socket = -1;
    }
    dns_task_handle = NULL;
    vTaskDelete(NULL);
}

// HTTP handler for GET / - Enhanced configuration interface
static esp_err_t enhanced_config_get_handler(httpd_req_t *req) {
    // Get current WiFi information
    char current_ssid[33] = "Not connected";
    char current_city[64] = "Not set";
    int8_t current_tz = 0;
    bool current_24h = true;
    int rssi = 0;

    // Load current settings from NVS
    nvs_config_load_location(current_city, &current_tz);
    nvs_config_load_time_format(&current_24h); // Ignore errors, use default

    // Get WiFi status
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret == ESP_OK) {
        snprintf(current_ssid, sizeof(current_ssid), "%s", (char *)ap_info.ssid);
        rssi = ap_info.rssi;
    }

    // Generate timezone and time format options with current selections
    const char *timezone_options = generate_timezone_options(current_tz);
    const char *timeformat_options = generate_timeformat_options(current_24h);

    // Create the complete HTML response
    char *html_response = malloc(8192);
    if (!html_response) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }

    snprintf(html_response, 8192, enhanced_config_html_template, current_ssid, rssi, current_city,
             current_tz,                             // Status section
             current_24h ? "24-hour" : "12-hour",    // Time format status
             current_city, current_city, current_tz, // Location form
             timezone_options,                       // Timezone options
             current_24h ? "24-hour" : "12-hour",    // Time format current value
             timeformat_options                      // Time format options
    );

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_response, HTTPD_RESP_USE_STRLEN);

    free(html_response);
    return ESP_OK;
}

// HTTP handler for POST /location - Update location and timezone only
static esp_err_t location_post_handler(httpd_req_t *req) {
    char content[256];
    size_t recv_size =
        (req->content_len < sizeof(content) - 1) ? req->content_len : sizeof(content) - 1;
    int ret = httpd_req_recv(req, content, recv_size);

    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request");
        return ESP_FAIL;
    }

    content[ret] = '\0';
    ESP_LOGI(TAG, "Received location data: %s", content);

    char city[MAX_CITY_LEN + 1] = {0};
    int8_t timezone = 0;
    bool use_24_hour = true;

    // Parse city, timezone, and time format from form data
    if (!parse_location_form_data(content, city, &timezone, &use_24_hour)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid form data");
        return ESP_FAIL;
    }

    // Validate inputs
    if (strlen(city) == 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "City cannot be empty");
        return ESP_FAIL;
    }

    // Save location to NVS
    esp_err_t err = nvs_config_save_location(city, timezone);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save location config: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save location");
        return ESP_FAIL;
    }

    // Save time format to NVS
    err = nvs_config_save_time_format(use_24_hour);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save time format config: %s", esp_err_to_name(err));
        // Don't fail the request for time format errors, just log
    }

    ESP_LOGI(TAG, "Location updated - City: '%s', TZ: UTC%+d, Format: %s", city, timezone,
             use_24_hour ? "24-hour" : "12-hour");

    // Send success response and redirect
    const char *success_response = "<!DOCTYPE html><html><head>"
                                   "<meta http-equiv='refresh' content='2;url=/'>"
                                   "<title>Settings Updated</title></head><body>"
                                   "<h2>✅ Location & Time Settings Updated!</h2>"
                                   "<p>City: %s</p><p>Timezone: UTC%+d</p><p>Time Format: %s</p>"
                                   "<p>Redirecting back to configuration...</p></body></html>";

    char response[512];
    snprintf(response, sizeof(response), success_response, city, timezone,
             use_24_hour ? "24-hour" : "12-hour");

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

// HTTP handler for POST /wifi - Update WiFi settings and restart
static esp_err_t wifi_post_handler(httpd_req_t *req) {
    char content[256];
    size_t recv_size =
        (req->content_len < sizeof(content) - 1) ? req->content_len : sizeof(content) - 1;
    int ret = httpd_req_recv(req, content, recv_size);

    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request");
        return ESP_FAIL;
    }

    content[ret] = '\0';
    ESP_LOGI(TAG, "Received WiFi data: %s", content);

    char ssid[MAX_SSID_LEN + 1] = {0};
    char password[MAX_PASS_LEN + 1] = {0};

    // Parse only SSID and password from form data
    if (!parse_wifi_form_data(content, ssid, password)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid form data");
        return ESP_FAIL;
    }

    // Validate inputs
    if (strlen(ssid) == 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "SSID cannot be empty");
        return ESP_FAIL;
    }

    // Save WiFi settings to NVS
    esp_err_t err = nvs_config_save_wifi(ssid, password);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save WiFi config: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save WiFi settings");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "WiFi settings updated - SSID: '%s'", ssid);

    // Send success response with restart instruction
    const char *wifi_success_response =
        "<!DOCTYPE html><html><head><title>WiFi Updated</title></head><body>"
        "<h2>✅ WiFi Settings Updated!</h2>"
        "<p>New SSID: %s</p>"
        "<p><strong>Please restart your device to apply WiFi changes.</strong></p>"
        "<p>Press the reset button on your ESP32 or power cycle it.</p>"
        "<button onclick='location.href=\"/\"'>Back to Configuration</button>"
        "</body></html>";

    char response[512];
    snprintf(response, sizeof(response), wifi_success_response, ssid);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

// HTTP handler for POST /factory-reset - Clear all settings
static esp_err_t factory_reset_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Factory reset requested");

    // Clear all NVS settings
    esp_err_t err = nvs_config_clear_all();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear NVS: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Factory reset failed");
        return ESP_FAIL;
    }

    // Send response and schedule restart
    const char *reset_response =
        "<!DOCTYPE html><html><head><title>Factory Reset</title></head><body>"
        "<h2>🔄 Factory Reset Complete</h2>"
        "<p>All settings have been cleared. The device will restart in 3 seconds.</p>"
        "<p>After restart, look for the \"AuthorClock\" WiFi network to configure the device.</p>"
        "<script>setTimeout(function(){document.body.innerHTML='<h2>Restarting device...</h2>';}, "
        "3000);</script>"
        "</body></html>";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, reset_response, HTTPD_RESP_USE_STRLEN);

    // Schedule restart after sending response
    ESP_LOGI(TAG, "Scheduling device restart in 3 seconds");
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();

    return ESP_OK;
}

// HTTP handler for POST /configure
static esp_err_t configure_post_handler(httpd_req_t *req) {
    char content[512];
    size_t recv_size =
        (req->content_len < sizeof(content) - 1) ? req->content_len : sizeof(content) - 1;
    int ret = httpd_req_recv(req, content, recv_size);

    if (ret <= 0) {
        ESP_LOGE(TAG, "Failed to receive POST data");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request");
        return ESP_FAIL;
    }

    content[ret] = '\0';
    ESP_LOGI(TAG, "Received form data: %s", content);

    char ssid[MAX_SSID_LEN + 1] = {0};
    char password[MAX_PASS_LEN + 1] = {0};
    char city[MAX_CITY_LEN + 1] = {0};
    int8_t timezone = 0;

    if (!parse_form_data(content, ssid, password, city, &timezone)) {
        ESP_LOGE(TAG, "Failed to parse form data");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid form data");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Parsed config - SSID: '%s', City: '%s', TZ: %+d", ssid, city, timezone);

    // Validate inputs
    if (strlen(ssid) == 0 || strlen(city) == 0) {
        ESP_LOGE(TAG, "SSID or city cannot be empty");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "SSID and city cannot be empty");
        return ESP_FAIL;
    }

    // Save to NVS
    esp_err_t err = nvs_config_save_wifi(ssid, password);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save WiFi config: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to save WiFi configuration");
        return ESP_FAIL;
    }

    err = nvs_config_save_location(city, timezone);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save location config: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to save location configuration");
        return ESP_FAIL;
    }

    // Check if this is persistent config mode (callback is NULL) or initial provisioning
    bool is_persistent_mode = (complete_callback == NULL);

    if (is_persistent_mode) {
        // In persistent mode, send update confirmation and instructions
        const char *update_html =
            "<!DOCTYPE html>"
            "<html><head>"
            "<meta charset='UTF-8'>"
            "<title>Configuration Updated</title>"
            "<style>body{font-family:Arial,sans-serif;margin:40px;background:#f5f5f5;}"
            ".container{background:white;padding:30px;border-radius:8px;max-width:500px;margin:0 "
            "auto;}"
            ".success{color:#28a745;} .warning{color:#ffc107;} .info{color:#17a2b8;}"
            "button{background:#007bff;color:white;padding:10px "
            "20px;border:none;border-radius:4px;cursor:pointer;margin:5px;}"
            "button:hover{background:#0056b3;}</style>"
            "</head><body>"
            "<div class='container'>"
            "<h2 class='success'>✓ Configuration Updated Successfully</h2>"
            "<p><strong>SSID:</strong> %s</p>"
            "<p><strong>Location:</strong> %s</p>"
            "<p><strong>Timezone:</strong> UTC%+d</p>"
            "<div class='info'>"
            "<p><strong>What happens next:</strong></p>"
            "<ul>"
            "<li>Settings have been saved to device memory</li>"
            "<li>WiFi connection will be updated on next restart</li>"
            "<li>Timezone will take effect immediately</li>"
            "</ul>"
            "</div>"
            "<div class='warning'>"
            "<p><strong>To apply WiFi changes:</strong></p>"
            "<p>Press the reset button on your ESP32 device, or power cycle it.</p>"
            "</div>"
            "<p><button onclick='location.reload()'>Make More Changes</button></p>"
            "</div>"
            "</body></html>";

        char response[1500];
        snprintf(response, sizeof(response), update_html, ssid, city, timezone);

        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

        ESP_LOGI(TAG, "Configuration updated in persistent mode - no restart triggered");
    } else {
        // In initial provisioning mode, use original success page and restart
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, success_html, HTTPD_RESP_USE_STRLEN);

        ESP_LOGI(TAG, "Configuration saved successfully, triggering completion callback");

        // Trigger completion callback after a brief delay
        if (complete_callback) {
            // Use a timer or task to call the callback after the HTTP response is sent
            complete_callback();
        }
    }

    return ESP_OK;
}

// Initialize WiFi AP mode
static esp_err_t wifi_init_ap(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ap_netif = esp_netif_create_default_wifi_ap();

    // Configure custom IP address for AP
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 4, 69);       // ESP32 IP
    IP4_ADDR(&ip_info.gw, 192, 168, 4, 69);       // Gateway (same as IP)
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0); // Subnet mask

    ESP_ERROR_CHECK(esp_netif_dhcps_stop(ap_netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ip_info));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(ap_netif));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap =
            {
                .ssid = WIFI_AP_SSID,
                .ssid_len = strlen(WIFI_AP_SSID),
                .channel = 1,
                .password = "",
                .max_connection = 4,
                .authmode = WIFI_AUTH_OPEN,
                .pmf_cfg =
                    {
                        .required = false,
                    },
            },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. SSID: %s, IP: %s", WIFI_AP_SSID, WIFI_AP_IP);
    return ESP_OK;
}

// Start HTTP server
esp_err_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting HTTP server on port: %d", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers for enhanced configuration interface
        httpd_uri_t main_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = enhanced_config_get_handler,
        };
        httpd_register_uri_handler(server, &main_uri);

        // Location form handler
        httpd_uri_t location_uri = {
            .uri = "/location",
            .method = HTTP_POST,
            .handler = location_post_handler,
        };
        httpd_register_uri_handler(server, &location_uri);

        // WiFi form handler
        httpd_uri_t wifi_uri = {
            .uri = "/wifi",
            .method = HTTP_POST,
            .handler = wifi_post_handler,
        };
        httpd_register_uri_handler(server, &wifi_uri);

        // Factory reset handler
        httpd_uri_t factory_reset_uri = {
            .uri = "/factory-reset",
            .method = HTTP_POST,
            .handler = factory_reset_handler,
        };
        httpd_register_uri_handler(server, &factory_reset_uri);

        // Keep legacy configure endpoint for backward compatibility
        httpd_uri_t configure_uri = {
            .uri = "/configure",
            .method = HTTP_POST,
            .handler = configure_post_handler,
        };
        httpd_register_uri_handler(server, &configure_uri);

        // Add captive portal detection endpoints
        httpd_uri_t captive_portal_uris[] = {
            {.uri = "/generate_204",
             .method = HTTP_GET,
             .handler = captive_portal_handler}, // Android
            {.uri = "/connecttest.txt",
             .method = HTTP_GET,
             .handler = captive_portal_handler}, // Windows
            {.uri = "/hotspot-detect.html",
             .method = HTTP_GET,
             .handler = captive_portal_handler}, // iOS/macOS
            {.uri = "/canonical.html",
             .method = HTTP_GET,
             .handler = captive_portal_handler}, // Ubuntu
            {.uri = "/success.txt",
             .method = HTTP_GET,
             .handler = captive_portal_handler}, // Firefox
            {.uri = "/ncsi.txt",
             .method = HTTP_GET,
             .handler = captive_portal_handler}, // Windows NCSI
        };

        for (int i = 0; i < sizeof(captive_portal_uris) / sizeof(captive_portal_uris[0]); i++) {
            httpd_register_uri_handler(server, &captive_portal_uris[i]);
        }

        return ESP_OK;
    }

    ESP_LOGE(TAG, "Error starting HTTP server");
    return ESP_FAIL;
}

// Stop HTTP server
static void stop_webserver(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}

esp_err_t wifi_prov_start(wifi_prov_complete_cb_t on_complete_cb) {
    if (provisioning_active) {
        ESP_LOGW(TAG, "Provisioning already active");
        return ESP_ERR_INVALID_STATE;
    }

    complete_callback = on_complete_cb;

    ESP_LOGI(TAG, "Starting WiFi provisioning mode");

    esp_err_t ret = wifi_init_ap();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi AP: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = start_webserver();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start web server: %s", esp_err_to_name(ret));
        esp_wifi_stop();
        return ret;
    }

    // Start DNS server for captive portal
    xTaskCreate(dns_server_task, "dns_server", 4096, NULL, 5, &dns_task_handle);

    provisioning_active = true;
    ESP_LOGI(TAG, "WiFi provisioning started successfully");
    ESP_LOGI(TAG, "Connect to WiFi network '%s' and visit http://%s", WIFI_AP_SSID, WIFI_AP_IP);

    return ESP_OK;
}

esp_err_t wifi_prov_stop(void) {
    if (!provisioning_active) {
        ESP_LOGW(TAG, "Provisioning not active");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Stopping WiFi provisioning mode");

    // Stop DNS server first
    provisioning_active = false;
    if (dns_task_handle) {
        // Close socket to wake up the task
        if (dns_socket != -1) {
            shutdown(dns_socket, SHUT_RDWR);
            close(dns_socket);
            dns_socket = -1;
        }
        // Wait for task to finish
        vTaskDelay(pdMS_TO_TICKS(100));
        dns_task_handle = NULL;
    }

    stop_webserver();
    esp_wifi_stop();
    esp_wifi_deinit();

    if (ap_netif) {
        esp_netif_destroy_default_wifi(ap_netif);
        ap_netif = NULL;
    }

    complete_callback = NULL;

    ESP_LOGI(TAG, "WiFi provisioning stopped");
    return ESP_OK;
}

bool wifi_prov_is_active(void) {
    return provisioning_active;
}

// Initialize mDNS service for device discovery
static esp_err_t init_mdns_service(void) {
    esp_err_t ret = mdns_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mDNS: %s", esp_err_to_name(ret));
        return ret;
    }

    // Set hostname
    ret = mdns_hostname_set("authorclock");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mDNS hostname: %s", esp_err_to_name(ret));
        return ret;
    }

    // Set instance name
    ret = mdns_instance_name_set("Author Clock Configuration");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mDNS instance name: %s", esp_err_to_name(ret));
        return ret;
    }

    // Add HTTP service
    ret = mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add mDNS HTTP service: %s", esp_err_to_name(ret));
        return ret;
    }

    // Add service text record
    mdns_txt_item_t service_txt_data[] = {{"device", "Author Clock"}, {"version", "1.0"}};
    ret = mdns_service_txt_set("_http", "_tcp", service_txt_data, 2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mDNS service TXT: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "mDNS service initialized - device available at authorclock.local");
    return ESP_OK;
}

esp_err_t wifi_simple_config_start(void) {
    ESP_LOGI(TAG, "Starting simple configuration server");

    // Start web server on current interface (no APSTA complexity)
    esp_err_t ret = start_webserver();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start config web server: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize mDNS service for device discovery
    ret = init_mdns_service();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "mDNS initialization failed, continuing without mDNS discovery");
        // Don't fail completely - device is still accessible by IP
    }

    // Set persistent mode (no callback = no restart after config change)
    complete_callback = NULL;
    provisioning_active = true;

    ESP_LOGI(TAG, "Configuration server started on port 80");
    ESP_LOGI(TAG, "Access via device's IP address on your WiFi network");
    ESP_LOGI(TAG, "Or visit http://authorclock.local for easy discovery");

    return ESP_OK;
}