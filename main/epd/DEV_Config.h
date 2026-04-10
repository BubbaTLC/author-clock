/*****************************************************************************
* | File      	:   DEV_Config.h
* | Author      :   Waveshare team (adapted for ESP-IDF)
* | Function    :   Hardware underlying interface
* | Info        :
*----------------
* |	This version:   V2.0 (ESP-IDF)
* | Date        :   2024-04-10
* | Info        :   Adapted from Arduino version for ESP-IDF
******************************************************************************/
#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include <stdint.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

/**
 * data
 **/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

/**
 * GPIO config for ESP32
 * Matches wiring from skill documentation:
 * SCK=13, MOSI=14, CS=15, RST=26, DC=27, BUSY=25
 **/
#define EPD_SCK_PIN  GPIO_NUM_13
#define EPD_MOSI_PIN GPIO_NUM_14
#define EPD_CS_PIN   GPIO_NUM_15
#define EPD_RST_PIN  GPIO_NUM_26
#define EPD_DC_PIN   GPIO_NUM_27
#define EPD_BUSY_PIN GPIO_NUM_25

#define GPIO_PIN_SET   1
#define GPIO_PIN_RESET 0

/**
 * GPIO read and write
 **/
#define DEV_Digital_Write(_pin, _value) gpio_set_level(_pin, _value)
#define DEV_Digital_Read(_pin) gpio_get_level(_pin)

/**
 * delay x ms
 **/
#define DEV_Delay_ms(__xms) vTaskDelay((__xms) / portTICK_PERIOD_MS)

/*------------------------------------------------------------------------------------------------------*/
UBYTE DEV_Module_Init(void);
void DEV_SPI_WriteByte(UBYTE data);
UBYTE DEV_SPI_ReadByte(void);
void DEV_SPI_Write_nByte(UBYTE *pData, UDOUBLE len);

#endif