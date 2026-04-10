/*****************************************************************************
* | File      	:   DEV_Config.c
* | Author      :   Waveshare team (adapted for ESP-IDF)
* | Function    :   Hardware underlying interface
* | Info        :
*----------------
* |	This version:   V2.0 (ESP-IDF)
* | Date        :   2024-04-10
* | Info        :   Adapted from Arduino version for ESP-IDF
******************************************************************************/
#include "DEV_Config.h"

static const char *TAG = "EPD_DEV";
static spi_device_handle_t spi_handle;

static void GPIO_Config(void)
{
    gpio_config_t io_conf = {};

    // Configure output pins
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << EPD_RST_PIN) | (1ULL << EPD_DC_PIN) | 
                          (1ULL << EPD_CS_PIN) | (1ULL << EPD_SCK_PIN) | 
                          (1ULL << EPD_MOSI_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Configure input pin
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << EPD_BUSY_PIN);
    io_conf.pull_up_en = 1;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Set initial states for clean startup
    gpio_set_level(EPD_CS_PIN, 1);    // CS high (inactive)
    gpio_set_level(EPD_SCK_PIN, 0);   // Clock low
    gpio_set_level(EPD_RST_PIN, 1);   // Reset not asserted 
    gpio_set_level(EPD_DC_PIN, 0);    // Command mode initially
    
    ESP_LOGI(TAG, "Initial BUSY pin state: %d", gpio_get_level(EPD_BUSY_PIN));
}

static void SPI_Config(void)
{
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,  // Not used for EPD
        .mosi_io_num = EPD_MOSI_PIN,
        .sclk_io_num = EPD_SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 2000000,  // 2MHz
        .mode = 0,                  // SPI mode 0
        .spics_io_num = EPD_CS_PIN,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle));
}

/******************************************************************************
function:	Module Initialize, configure GPIO pins and SPI
parameter:
Info:
******************************************************************************/
UBYTE DEV_Module_Init(void)
{
    ESP_LOGI(TAG, "Initializing EPD hardware");
    
    GPIO_Config();
    SPI_Config();

    ESP_LOGI(TAG, "EPD hardware initialization complete");
    return 0;
}

/******************************************************************************
function: SPI write byte
******************************************************************************/
void DEV_SPI_WriteByte(UBYTE data)
{
    spi_transaction_t t = {
        .length = 8,
        .tx_data = {data},
        .flags = SPI_TRANS_USE_TXDATA
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_handle, &t));
}

/******************************************************************************
function: SPI read byte (not commonly used for EPD)
******************************************************************************/
UBYTE DEV_SPI_ReadByte(void)
{
    spi_transaction_t t = {
        .length = 8,
        .flags = SPI_TRANS_USE_RXDATA
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_handle, &t));
    return t.rx_data[0];
}

/******************************************************************************
function: SPI write multiple bytes
******************************************************************************/
void DEV_SPI_Write_nByte(UBYTE *pData, UDOUBLE len)
{
    if (len == 0) return;
    
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = pData
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_handle, &t));
}