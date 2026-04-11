#ifndef __FONTS_H
#define __FONTS_H

#include <stdint.h>

/* Fixed-width monochrome font descriptor used by the GUI_Paint EPD driver. */
typedef struct {
    const uint8_t *table;
    uint16_t Width;
    uint16_t Height;
} sFONT;

#endif /* __FONTS_H */
