/*****************************************************************************
* | File      	:	EPD_7in5_V2.h
* | Author      :   Waveshare team (adapted for ESP-IDF)
* | Function    :   Electronic paper driver
* | Info        :
*----------------
* |	This version:   V4.0 (ESP-IDF)
* | Date        :   2024-04-10
* | Info        :   Adapted from Arduino version for ESP-IDF
******************************************************************************/
#ifndef _EPD_7IN5_V2_H_
#define _EPD_7IN5_V2_H_

#include "DEV_Config.h"

// Display resolution
#define EPD_7IN5_V2_WIDTH       800
#define EPD_7IN5_V2_HEIGHT      480

UBYTE EPD_7IN5_V2_Init(void);
UBYTE EPD_7IN5_V2_Init_Fast(void);
UBYTE EPD_7IN5_V2_Init_Part(void);
void EPD_7IN5_V2_Clear(void);
void EPD_7IN5_V2_Display(UBYTE *blackimage);
void EPD_7IN5_V2_Display_Part(UBYTE *blackimage, UDOUBLE x_start, UDOUBLE y_start, UDOUBLE x_end, UDOUBLE y_end);
void EPD_7IN5_V2_Sleep(void);

#endif