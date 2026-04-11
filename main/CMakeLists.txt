idf_component_register(SRCS "main.c"
                           "nvs_config.c"
                           "wifi_prov.c"
                           "wifi_sta.c"
                           "ntp_sync.c"
                           "quote_fetcher.c"
                           "quote_reader.c"
                           "display_mgr.c"
                           "bmfont_renderer.c"
                           "fonts/QuicksandBook-Regular-48.c"
                           "epd/EPD_7in5_V2.c"
                           "epd/GUI_Paint.c"
                           "epd/DEV_Config.c"
                           "epd/font12.c"
                           "epd/font20.c"
                           "epd/font24.c"
                    INCLUDE_DIRS "." "epd" "fonts"
                    REQUIRES nvs_flash
                             esp_driver_gpio
                             esp_driver_spi
                             esp_timer
                             esp_wifi
                             esp_netif
                             esp_http_server
                             esp_http_client
                             esp-tls
                             esp_event
                             lwip
)
