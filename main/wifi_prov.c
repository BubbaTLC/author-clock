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

// HTML form for provisioning
static const char *provisioning_html =
    "<!DOCTYPE html>"
    "<html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Author Clock Setup</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;max-width:400px;margin:50px "
    "auto;padding:20px;background:#f5f5f5}"
    ".container{background:white;padding:30px;border-radius:8px;box-shadow:0 2px 10px "
    "rgba(0,0,0,0.1)}"
    "h1{color:#333;text-align:center;margin-bottom:30px}"
    ".form-group{margin-bottom:20px}"
    "label{display:block;margin-bottom:5px;color:#555;font-weight:bold}"
    "input,select{width:100%;padding:10px;border:1px solid "
    "#ddd;border-radius:4px;font-size:16px;box-sizing:border-box}"
    "input:focus,select:focus{outline:none;border-color:#4CAF50}"
    "button{width:100%;padding:12px;background:#4CAF50;color:white;border:none;border-radius:4px;"
    "font-size:16px;cursor:pointer}"
    "button:hover{background:#45a049}"
    ".info{background:#e8f4f8;padding:15px;border-radius:4px;margin-bottom:20px;font-size:14px;"
    "color:#666}"
    "</style>"
    "</head><body>"
    "<div class='container'>"
    "<h1>📚 Author Clock Setup</h1>"
    "<div class='info'>"
    "Configure your WiFi network and location settings. The device will restart after successful "
    "configuration."
    "</div>"
    "<form method='POST' action='/configure'>"
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
    "<div class='form-group'>"
    "<label for='city'>City for Weather:</label>"
    "<input type='text' id='city' name='city' required maxlength='63' placeholder='London, New "
    "York, Tokyo...'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='timezone'>Timezone Offset (hours from UTC):</label>"
    "<select id='timezone' name='timezone' required>"
    "<option value='-12'>UTC-12</option>"
    "<option value='-11'>UTC-11</option>"
    "<option value='-10'>UTC-10</option>"
    "<option value='-9'>UTC-9</option>"
    "<option value='-8'>UTC-8</option>"
    "<option value='-7'>UTC-7</option>"
    "<option value='-6'>UTC-6</option>"
    "<option value='-5'>UTC-5</option>"
    "<option value='-4'>UTC-4</option>"
    "<option value='-3'>UTC-3</option>"
    "<option value='-2'>UTC-2</option>"
    "<option value='-1'>UTC-1</option>"
    "<option value='0' selected>UTC+0 (London)</option>"
    "<option value='1'>UTC+1</option>"
    "<option value='2'>UTC+2</option>"
    "<option value='3'>UTC+3</option>"
    "<option value='4'>UTC+4</option>"
    "<option value='5'>UTC+5</option>"
    "<option value='6'>UTC+6</option>"
    "<option value='7'>UTC+7</option>"
    "<option value='8'>UTC+8</option>"
    "<option value='9'>UTC+9</option>"
    "<option value='10'>UTC+10</option>"
    "<option value='11'>UTC+11</option>"
    "<option value='12'>UTC+12</option>"
    "<option value='13'>UTC+13</option>"
    "<option value='14'>UTC+14</option>"
    "</select>"
    "</div>"
    "<button type='submit'>Configure Author Clock</button>"
    "</form>"
    "</div>"
    "</body></html>";

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

// HTTP handler for GET /
static esp_err_t provisioning_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, provisioning_html, HTTPD_RESP_USE_STRLEN);
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

    // Send success page
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, success_html, HTTPD_RESP_USE_STRLEN);

    ESP_LOGI(TAG, "Configuration saved successfully, triggering completion callback");

    // Trigger completion callback after a brief delay
    if (complete_callback) {
        // Use a timer or task to call the callback after the HTTP response is sent
        complete_callback();
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
static esp_err_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting HTTP server on port: %d", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        httpd_uri_t provisioning_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = provisioning_get_handler,
        };
        httpd_register_uri_handler(server, &provisioning_uri);

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