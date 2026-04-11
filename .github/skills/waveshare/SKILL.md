---
name: waveshare
description: "Program the Waveshare 7.5-inch V2 e-paper display (EPD_7IN5_V2, 800x480). Use when: writing display code, drawing text/shapes/bitmaps to e-ink, setting up SPI wiring, choosing refresh modes (normal/fast/partial/4-gray), allocating image buffers, or debugging blank/frozen screens. Includes full API reference and wiring pinout."
argument-hint: "Describe what you want to display (text, image, shapes, clock, partial refresh, etc.)"
---

# Waveshare 7.5-inch V2 E-Paper Display

## Display Specs
- Resolution: **800 × 480** pixels (1-bit black/white, or 2-bit 4-gray)
- Interface: SPI
- Driver files: `EPD_7in5_V2.h/.cpp`, `DEV_Config.h/.cpp`, `GUI_Paint.h/.cpp`
- Demo reference: [`epd7in5_V2-demo.ino`](./epd7in5_V2-demo/epd7in5_V2-demo.ino)

---

## Wiring (Arduino / ESP32)

| Signal  | Pin |
|---------|-----|
| SCK     | 13  |
| MOSI    | 14  |
| CS      | 15  |
| RST     | 26  |
| DC      | 27  |
| BUSY    | 25  |

Defined in [`DEV_Config.h`](./epd7in5_V2-demo/DEV_Config.h). Change these macros to match your board.

---

## Core Workflow

### 1. Initialize hardware
```cpp
DEV_Module_Init();
```

### 2. Choose a refresh mode and init the display

| Mode    | Function                   | Use For                                   |
| ------- | -------------------------- | ----------------------------------------- |
| Normal  | `EPD_7IN5_V2_Init()`       | Full, high-quality refresh                |
| Fast    | `EPD_7IN5_V2_Init_Fast()`  | Quicker full refresh (slight ghosting OK) |
| Partial | `EPD_7IN5_V2_Init_Part()`  | Update a small region without full flash  |
| 4-Gray  | `EPD_7IN5_V2_Init_4Gray()` | Grayscale rendering (4 levels)            |

### 3. Clear the display (optional but recommended on first boot)
```cpp
EPD_7IN5_V2_Clear();
DEV_Delay_ms(500);
```

### 4. Allocate an image buffer
```cpp
UBYTE *BlackImage;
UWORD Imagesize = ((EPD_7IN5_V2_WIDTH % 8 == 0)
    ? (EPD_7IN5_V2_WIDTH / 8)
    : (EPD_7IN5_V2_WIDTH / 8 + 1)) * EPD_7IN5_V2_HEIGHT;

BlackImage = (UBYTE *)malloc(Imagesize);
if (BlackImage == NULL) { /* handle OOM */ }
```

### 5. Create and set up the canvas
```cpp
Paint_NewImage(BlackImage, EPD_7IN5_V2_WIDTH, EPD_7IN5_V2_HEIGHT, ROTATE_0, WHITE);
Paint_SelectImage(BlackImage);
Paint_Clear(WHITE);
```

### 6. Draw content (see Drawing API below)

### 7. Push buffer to display
```cpp
EPD_7IN5_V2_Display(BlackImage);   // full refresh
// or for partial:
EPD_7IN5_V2_Display_Part(BlackImage, x0, y0, x1, y1);
```

### 8. Sleep the display when done
```cpp
EPD_7IN5_V2_Sleep();
free(BlackImage);
```
> Always call `Sleep()` — leaving the display powered without refreshing damages it over time.

---

## Drawing API (GUI_Paint)

### Canvas setup
```cpp
Paint_NewImage(image, width, height, rotate, color);
Paint_SelectImage(image);
Paint_Clear(WHITE);                      // fill canvas
Paint_ClearWindows(x0, y0, x1, y1, WHITE); // fill region
Paint_SetRotate(ROTATE_0 | ROTATE_90 | ROTATE_180 | ROTATE_270);
Paint_SetMirroring(MIRROR_NONE | MIRROR_HORIZONTAL | MIRROR_VERTICAL);
```

### Primitives
```cpp
Paint_DrawPoint(x, y, color, DOT_PIXEL_1X1, DOT_STYLE_DFT);
Paint_DrawLine(x0, y0, x1, y1, color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
Paint_DrawRectangle(x0, y0, x1, y1, color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
Paint_DrawCircle(cx, cy, r, color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
```

### Text
```cpp
// Available fonts: Font12, Font16, Font20, Font24 (EN); Font12CN, Font24CN (CN/GB2312)
Paint_DrawString_EN(x, y, "text", &Font16, foreColor, backColor);
Paint_DrawString_CN(x, y, "中文", &Font24CN, foreColor, backColor);
Paint_DrawNum(x, y, number, &Font16, foreColor, backColor);
Paint_DrawTime(x, y, &sPaint_time, &Font20, foreColor, backColor);
```

### Bitmaps
```cpp
Paint_DrawBitMap(gImage_7in5_V2);  // full-screen raw bitmap array (stored in ImageData.c)
```

### 4-Gray mode colors
```cpp
// Use with EPD_7IN5_V2_Init_4Gray() + EPD_7IN5_V2_Display_4Gray()
GRAY1  // darkest black
GRAY2
GRAY3
GRAY4  // white
```

---

## Partial Refresh Pattern (e.g., clock)

```cpp
EPD_7IN5_V2_Init_Part();
// Create a small buffer for just the region being updated
Paint_NewImage(BlackImage, Font20.Width * 7, Font20.Height, 0, WHITE);
Paint_SelectImage(BlackImage);

PAINT_TIME t = {.Hour=12, .Min=0, .Sec=0};
for (;;) {
    // increment time ...
    Paint_ClearWindows(0, 0, Font20.Width * 7, Font20.Height, WHITE);
    Paint_DrawTime(0, 0, &t, &Font20, WHITE, BLACK);
    EPD_7IN5_V2_Display_Part(BlackImage, 0, 0, Font20.Width * 7, Font20.Height);
    DEV_Delay_ms(1000);
}
```

---

## Common Issues

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| Blank / all-white screen | Display not initialized or buffer not flushed | Confirm `EPD_7IN5_V2_Init()` completes; check BUSY pin wiring |
| Ghosting / grey artefacts | Skipped full clear after changing content | Call `EPD_7IN5_V2_Clear()` before switching modes |
| Crash / freeze on malloc | Heap too small | Increase heap; use PSRAM on ESP32 (`ps_malloc`) |
| Partial refresh corrupts image | Wrong buffer size for the region | Buffer width/height must match the partial region dimensions exactly |
| Nothing drawn after `Display()` | Wrong image pointer passed | Verify `Paint_SelectImage()` and `EPD_7IN5_V2_Display()` use the same pointer |

---

## File Reference

| File | Purpose |
|------|---------|
| [`DEV_Config.h`](./epd7in5_V2-demo/DEV_Config.h) | Pin definitions, SPI helpers, type aliases |
| [`EPD_7in5_V2.h`](./epd7in5_V2-demo/EPD_7in5_V2.h) | Display driver API |
| [`GUI_Paint.h`](./epd7in5_V2-demo/GUI_Paint.h) | Drawing canvas API |
| [`fonts.h`](./epd7in5_V2-demo/fonts.h) | Font declarations |
| [`ImageData.h`](./epd7in5_V2-demo/ImageData.h) | Bitmap array declarations |
| [`epd7in5_V2-demo.ino`](./epd7in5_V2-demo/epd7in5_V2-demo.ino) | Full working demo sketch |


