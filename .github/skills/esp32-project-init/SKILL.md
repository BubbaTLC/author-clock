---
name: esp32-project-init
description: Initialize a new ESP32 project with proper structure and configuration using ESP-IDF. Use when creating ESP32 embedded projects, IoT devices, or microcontroller applications.
allowed-tools: fs_read fs_write execute_bash
metadata:
  author: kiro-cli
  version: "1.0"
  category: embedded
  compatibility: Requires ESP-IDF installed and configured
---

# ESP32 Project Initialization

## Instructions

1. **Set up ESP-IDF environment**
```bash
. $HOME/esp/esp-idf/export.sh
```

2. **Create project from template**
```bash
mkdir -p ~/esp32-projects && cd ~/esp32-projects
idf.py create-project <project_name>
cd <project_name>
```

3. **Set target chip**
```bash
idf.py set-target esp32  # or esp32s2, esp32s3, esp32c3
```

4. **Create project structure**
```bash
mkdir -p components main/include
```

5. **Create main application**
```c
// main/main.c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting application...");
    
    while (1) {
        ESP_LOGI(TAG, "Hello from ESP32!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

6. **Configure CMakeLists.txt**
```cmake
# CMakeLists.txt (root)
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(<project_name>)
```

```cmake
# main/CMakeLists.txt
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "include"
)
```

7. **Build and flash**
```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Examples

### Basic Hello World
```bash
. ~/esp/esp-idf/export.sh
idf.py create-project hello_esp32
cd hello_esp32
idf.py set-target esp32
idf.py build
idf.py -p /dev/cu.usbserial-0001 flash monitor
```

### From WiFi Example
```bash
cp -r $IDF_PATH/examples/wifi/getting_started/station my_wifi_project
cd my_wifi_project
idf.py set-target esp32
idf.py menuconfig
idf.py build flash monitor
```

## Troubleshooting

- **ESP-IDF not found**: Source the export script: `. $HOME/esp/esp-idf/export.sh`
- **Target not set**: Run `idf.py set-target esp32` before building
- **Port permission denied**: Add user to dialout group (Linux) or check USB drivers (macOS)
- **Build fails after target change**: Run `idf.py fullclean` then rebuild
