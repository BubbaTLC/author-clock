#ifndef __FONTS_H
#define __FONTS_H

#include <stdint.h>

/* Includes ------------------------------------------------------------------*/
//ASCII
typedef struct _tFont
{    
  const uint8_t *table;
  uint16_t Width;
  uint16_t Height;
  
} sFONT;

//GB2312
#define MAX_HEIGHT_FONT         41
#define MAX_WIDTH_FONT          32

typedef struct                                          // 汉字字模数据结构
{
  unsigned char index[3];                               // 汉字内码索引
  const char matrix[MAX_HEIGHT_FONT*MAX_WIDTH_FONT/8];  // 点阵码数据
}CH_CN;

typedef struct
{    
  const CH_CN *table;
  uint16_t size;
  uint16_t ASCII_Width;
  uint16_t Width;
  uint16_t Height;
  
}cFONT;

// Font declarations - defined in separate font files
extern sFONT Font12;  // 7x12 pixels - for time
extern sFONT Font20;  // 14x20 pixels - for title/author  
extern sFONT Font24;  // 17x24 pixels - for quote text

#endif /* __FONTS_H */