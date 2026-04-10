# Author Clock - ESP32 E-Paper Book Quote Display

An ESP32-based book quote clock that displays literary quotes on a 7.5" e-paper display, with each quote changing every minute based on the time of day.

## Features

- **7.5" Waveshare E-Paper Display** (EPD_7IN5_V2) - 800x480 resolution
- **WiFi Connectivity** - Easy setup via captive portal
- **Time Synchronization** - NTP-based with timezone support
- **Literary Quotes** - 1440+ quotes indexed by time of day
- **Low Power** - E-paper display updates only when content changes

## Hardware Requirements

### Components
- ESP32 development board
- Waveshare 7.5" e-Paper display (V2) - Model EPD_7IN5_V2
- Jumper wires for connections

### Wiring (ESP32 to EPD)
```
ESP32    →  EPD Pin
GPIO13   →  SCK (Clock)
GPIO14   →  MOSI (Data)  
GPIO15   →  CS (Chip Select)
GPIO26   →  RST (Reset)
GPIO27   →  DC (Data/Command)
GPIO25   →  BUSY (Busy Status)
3.3V     →  VCC
GND      →  GND
```

## Quick Start

### 1. Build and Flash

```bash
# Set up ESP-IDF environment
. $HOME/.espressif/v6.0/esp-idf/export.sh

# Clone and build
cd book-clock
idf.py set-target esp32
idf.py build flash monitor
```

### 2. Initial Setup (Provisioning)

On first boot, the device creates a WiFi access point:

1. Connect to **"AuthorClock"** WiFi network
2. Open browser to `http://192.168.4.1`
3. Enter your WiFi credentials
4. Enter your location and timezone offset:
   - **Saskatoon, SK**: `-6` (UTC-6)
   - **Vancouver, BC**: `-8` (UTC-8)
   - **Toronto, ON**: `-5` (UTC-5)
   - **London, UK**: `0` (UTC+0)
5. Submit the form

The device will restart and connect to your WiFi network.

### 3. Operation

- Display updates every minute with time-appropriate quotes
- Quotes are stored locally on the device (no internet required after setup)
- Time synchronizes with NTP servers periodically

## Troubleshooting

### Configuration Reset

**Easy Method: Web Interface (Always Available)**

The device runs a configuration web server on your WiFi network:

1. Ensure the clock is connected to your WiFi network
2. Find the device's IP address from your router or device logs
3. Open browser to `http://[device-ip-address]` (e.g., `http://192.168.1.100`)
4. Enter new WiFi credentials and/or timezone:
   - **Saskatoon, SK**: `-6` (UTC-6)
   - **Vancouver, BC**: `-8` (UTC-8)  
   - **Toronto, ON**: `-5` (UTC-5)
5. Submit form - settings are saved immediately
6. For WiFi changes: Reset device (press RST button) to connect to new network

**Alternative: Hardware Reset (if web interface unavailable)**

If you can't access the WiFi network for some reason:

1. Hold down the **BOOT** button on your ESP32
2. Press and release the **EN/RST** button (while holding BOOT)
3. Keep holding **BOOT for 3+ seconds** after reset
4. Release BOOT button - device clears settings and restarts into setup mode

### Common Issues

#### Wrong Time Display
- **Problem**: Clock shows wrong time (e.g., 4 AM instead of 4 PM)
- **Cause**: Incorrect timezone offset during setup
- **Solution**: Use reset method above and re-enter correct timezone
  - For Saskatoon: Enter `-6` (negative six)

#### E-Paper Display Issues
- **Problem**: Display timeouts or blank screen
- **Cause**: Wiring issues or timing problems
- **Solution**: 
  - Verify wiring connections
  - Check power supply (3.3V, sufficient current)
  - Monitor serial output for error messages

#### WiFi Connection Failed
- **Problem**: Cannot connect to WiFi
- **Cause**: Wrong credentials or weak signal
- **Solution**: Reset configuration and re-enter credentials

### Serial Monitoring

Monitor device logs with timestamps:
```bash
./tools/monitor.sh
```

This shows real-time output with wall-clock timestamps for easier debugging.

## Development

### Project Structure
```
├── main/               # Main application code
│   ├── epd/           # E-paper display driver
│   ├── main.c         # Application entry point
│   ├── display_mgr.c  # Display management
│   ├── ntp_sync.c     # Time synchronization  
│   ├── wifi_*.c       # WiFi connectivity
│   └── quote_*.c      # Quote storage and retrieval
├── data/              # Quote data files
├── tools/             # Utility scripts
└── partitions.csv     # Flash partition layout
```

### Adding Quotes

Quotes are stored in a binary format for efficient access. To update:

1. Edit `data.json` with new quotes (JSON format)
2. Run `python tools/gen_quotes_bin.py` to generate binary file
3. Flash the updated partition: `idf.py partition-table-flash`

### Custom Timezone Handling

The project uses POSIX timezone format internally:
- For UTC-N timezones, enter `-N` during setup
- For UTC+N timezones, enter `+N` during setup

## Hardware Notes

### E-Paper Display Timing
- Full refresh takes ~4 seconds
- Display enters deep sleep between updates
- BUSY pin timing is critical for proper operation

### Power Consumption
- Active (updating): ~200mA
- Idle (WiFi connected): ~100mA  
- Deep sleep potential: <10mA (not currently implemented)

## License

This project contains code adapted from:
- Waveshare e-Paper library (originally for Arduino, adapted for ESP-IDF)
- ESP-IDF examples and components

See individual files for specific license information.

## Contributing

1. Follow ESP-IDF coding standards
2. Use `ESP_LOGI/LOGW/LOGE` for logging (not printf)
3. Add proper error handling with `ESP_ERROR_CHECK()`
4. Test with actual hardware before submitting PRs