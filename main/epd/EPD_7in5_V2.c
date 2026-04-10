/*****************************************************************************
* | File      	:	EPD_7in5_V2.c
* | Author      :   Waveshare team (adapted for ESP-IDF)
* | Function    :   Electronic paper driver
* | Info        :
*----------------
* |	This version:   V4.0 (ESP-IDF)
* | Date        :   2024-04-10
* | Info        :   Adapted from Arduino version for ESP-IDF
******************************************************************************/
#include "EPD_7in5_V2.h"

static const char *TAG = "EPD_7IN5_V2";

#define EPD_BUSY_TIMEOUT_MS  30000

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_Reset(void)
{
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(20);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(2);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(20);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void EPD_SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void EPD_SendData(UBYTE Data)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

static void EPD_SendData2(UBYTE *pData, UDOUBLE len)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_Write_nByte(pData, len);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
static void EPD_WaitUntilIdle(void)
{
    ESP_LOGI(TAG, "e-Paper busy");
    uint32_t elapsed = 0;
    while (DEV_Digital_Read(EPD_BUSY_PIN) == 0) {
        DEV_Delay_ms(20);   // 2 ticks — yields to IDLE so TWDT doesn't fire
        elapsed += 20;
        if (elapsed >= EPD_BUSY_TIMEOUT_MS) {
            ESP_LOGE(TAG, "e-Paper busy timeout after %lu ms — BUSY pin never went HIGH", (unsigned long)elapsed);
            return;
        }
    }
    DEV_Delay_ms(5);
    ESP_LOGI(TAG, "e-Paper busy release after %lu ms", (unsigned long)elapsed);
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
static void EPD_7IN5_V2_TurnOnDisplay(void)
{	
    EPD_SendCommand(0x12);			//DISPLAY REFRESH
    DEV_Delay_ms(100);	        //!!!The delay here is necessary, 200uS at least!!!
    EPD_WaitUntilIdle();
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
UBYTE EPD_7IN5_V2_Init(void)
{
    ESP_LOGI(TAG, "Initializing EPD 7.5 inch V2");
    
    EPD_Reset();
    
    EPD_SendCommand(0x01);			//POWER SETTING
	EPD_SendData(0x07);
	EPD_SendData(0x07);    //VGH=20V,VGL=-20V
	EPD_SendData(0x3f);		//VDH=15V
	EPD_SendData(0x3f);		//VDL=-15V

	//Enhanced display drive(Add 0x06 command)
	EPD_SendCommand(0x06);			//Booster Soft Start 
	EPD_SendData(0x17);
	EPD_SendData(0x17);   
	EPD_SendData(0x28);		
	EPD_SendData(0x17);	

	EPD_SendCommand(0x04); //POWER ON
	DEV_Delay_ms(100); 
	EPD_WaitUntilIdle();        //waiting for the electronic paper IC to release the idle signal

	EPD_SendCommand(0X00);			//PANNEL SETTING
	EPD_SendData(0x1F);   //KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f

	EPD_SendCommand(0x61);        	//tres			
	EPD_SendData(0x03);		//source 800
	EPD_SendData(0x20);
	EPD_SendData(0x01);		//gate 480
	EPD_SendData(0xE0);  

	EPD_SendCommand(0X15);		
	EPD_SendData(0x00);		

	EPD_SendCommand(0X50);			
	EPD_SendData(0x10);
	EPD_SendData(0x07);

	EPD_SendCommand(0X60);			
	EPD_SendData(0x22);
	
    ESP_LOGI(TAG, "EPD initialization complete");
    return 0;
}

/******************************************************************************
function :	Initialize the e-Paper register (Fast mode)
parameter:
******************************************************************************/
UBYTE EPD_7IN5_V2_Init_Fast(void)
{
    ESP_LOGI(TAG, "Initializing EPD 7.5 inch V2 (Fast mode)");
    
    EPD_Reset();
    
    EPD_SendCommand(0x01);			//POWER SETTING
	EPD_SendData(0x07);
	EPD_SendData(0x07);    
	EPD_SendData(0x3f);		
	EPD_SendData(0x3f);		

	EPD_SendCommand(0x06);			//Booster Soft Start 
	EPD_SendData(0x17);
	EPD_SendData(0x17);   
	EPD_SendData(0x28);		
	EPD_SendData(0x17);	

	EPD_SendCommand(0x04); //POWER ON
	DEV_Delay_ms(100); 
	EPD_WaitUntilIdle();        

	EPD_SendCommand(0X00);			//PANNEL SETTING
	EPD_SendData(0x1F);   

	EPD_SendCommand(0x61);        	//tres			
	EPD_SendData(0x03);		//source 800
	EPD_SendData(0x20);
	EPD_SendData(0x01);		//gate 480
	EPD_SendData(0xE0);  

	EPD_SendCommand(0X15);		
	EPD_SendData(0x00);		

	EPD_SendCommand(0X50);			
	EPD_SendData(0x10);
	EPD_SendData(0x00);

	EPD_SendCommand(0X60);			
	EPD_SendData(0x22);
	
    ESP_LOGI(TAG, "EPD fast mode initialization complete");
    return 0;
}

/******************************************************************************
function :	Initialize the e-Paper register (Partial refresh mode)
parameter:
******************************************************************************/
UBYTE EPD_7IN5_V2_Init_Part(void)
{
    ESP_LOGI(TAG, "Initializing EPD 7.5 inch V2 (Partial mode)");
    
    EPD_Reset();
    
    EPD_SendCommand(0x01);			//POWER SETTING
	EPD_SendData(0x07);
	EPD_SendData(0x07);    
	EPD_SendData(0x3f);		
	EPD_SendData(0x3f);		

	EPD_SendCommand(0x06);			//Booster Soft Start 
	EPD_SendData(0x17);
	EPD_SendData(0x17);   
	EPD_SendData(0x28);		
	EPD_SendData(0x17);	

	EPD_SendCommand(0x04); //POWER ON
	DEV_Delay_ms(100); 
	EPD_WaitUntilIdle();        

	EPD_SendCommand(0X00);			//PANNEL SETTING
	EPD_SendData(0x1F);   

	EPD_SendCommand(0x61);        	//tres			
	EPD_SendData(0x03);		//source 800
	EPD_SendData(0x20);
	EPD_SendData(0x01);		//gate 480
	EPD_SendData(0xE0);  

	EPD_SendCommand(0X15);		
	EPD_SendData(0x00);		

	EPD_SendCommand(0X50);			
	EPD_SendData(0x11);
	EPD_SendData(0x07);

	EPD_SendCommand(0X60);			
	EPD_SendData(0x22);
	
    ESP_LOGI(TAG, "EPD partial mode initialization complete");
    return 0;
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_7IN5_V2_Clear(void)
{
    UWORD Width, Height;
    Width = (EPD_7IN5_V2_WIDTH % 8 == 0)? (EPD_7IN5_V2_WIDTH / 8 ): (EPD_7IN5_V2_WIDTH / 8 + 1);
    Height = EPD_7IN5_V2_HEIGHT;
    
    ESP_LOGI(TAG, "Clearing display");

    // DTM1 (old frame): bit=1 → white; 0xFF = all white
    EPD_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(0xFF);
        }
    }

    // DTM2 (new frame): bit=0 → white (inverted polarity vs DTM1); 0x00 = all white
    EPD_SendCommand(0x13);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(0x00);
        }
    }
    
    EPD_7IN5_V2_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_7IN5_V2_Display(UBYTE *blackimage)
{
    UWORD Width, Height;
    Width = (EPD_7IN5_V2_WIDTH % 8 == 0)? (EPD_7IN5_V2_WIDTH / 8 ): (EPD_7IN5_V2_WIDTH / 8 + 1);
    Height = EPD_7IN5_V2_HEIGHT;

    ESP_LOGI(TAG, "Displaying image");

    // DTM1 (old frame): bit=1 → white, bit=0 → black — send image as-is
    EPD_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(blackimage[i + j * Width]);
        }
    }

    // DTM2 (new frame): inverted polarity vs DTM1 — must send bitwise-NOT
    EPD_SendCommand(0x13);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(~blackimage[i + j * Width]);
        }
    }

    EPD_7IN5_V2_TurnOnDisplay();
}

/******************************************************************************
function :	Partial refresh
parameter:
******************************************************************************/
void EPD_7IN5_V2_Display_Part(UBYTE *blackimage, UDOUBLE x_start, UDOUBLE y_start, UDOUBLE x_end, UDOUBLE y_end)
{
    ESP_LOGI(TAG, "Partial display refresh (%ld,%ld) to (%ld,%ld)", x_start, y_start, x_end, y_end);
    
    // For simplicity, fall back to full refresh
    // Full partial refresh implementation would require more complex logic
    EPD_7IN5_V2_Display(blackimage);
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_7IN5_V2_Sleep(void)
{
    ESP_LOGI(TAG, "Entering sleep mode");
    
    EPD_SendCommand(0x02); // POWER_OFF
    EPD_WaitUntilIdle();
    EPD_SendCommand(0x07); // DEEP_SLEEP
    EPD_SendData(0XA5);
}