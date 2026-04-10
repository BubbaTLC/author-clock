#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// GitHub URL for quotes data
#define GITHUB_QUOTES_URL                                                                          \
    "https://raw.githubusercontent.com/ambercaravalho/open-author-clock/main/data.json"

// NTP server
#define NTP_SERVER "pool.ntp.org"

// OpenWeatherMap API settings
#define OWM_API_URL_FMT "https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric"

// WiFi AP settings for provisioning
#define WIFI_AP_SSID "AuthorClock"
#define WIFI_AP_IP "192.168.4.69"

// Update intervals
#define WEATHER_UPDATE_INTERVAL_S 1800 // 30 minutes
#define CLOCK_UPDATE_INTERVAL_S 60     // 1 minute

// Task stack sizes
#define CLOCK_TASK_STACK_SIZE 8192
#define WEATHER_TASK_STACK_SIZE 4096

// Maximum retry attempts for WiFi connection
#define MAX_WIFI_RETRY 5

// DNS server settings for captive portal
#define DNS_PORT 53
#define DNS_MAX_LEN 512

#endif // APP_CONFIG_H