---
name: display-spec
description: "Hardware specifications for the Waveshare 7.5-inch V2 e-paper display (EPD_7IN5_V2). Use when: looking up display dimensions, resolution, refresh times, power consumption, operating temperature, pixel encoding, precautions, SPI interface pins, electrical ratings, absolute maximum ratings, command table, or operating sequence. Full datasheet available in spec reference."
argument-hint: "Ask about a spec: resolution, refresh time, power, dimensions, pixel encoding, precautions, SPI pins, commands, electrical characteristics, etc."
---

# Waveshare 7.5-inch e-Paper HAT — Display Specification

> Full datasheet: [spec/spec.md](./spec/spec.md) — includes command table, electrical characteristics, timing diagrams, operating sequence, and optical specs.

Source: https://www.waveshare.com/wiki/7.5inch_e-Paper_HAT_Manual#Overview

---

## Version

Resolution: **800 × 480**. Supports partial and fast refresh. Versions sold after Sept 2023 use `7.5V2`; versions sold before Sept 2023 use `7.5V2_old`.

---

## Parameters

| Parameter                   | Value                                            |
| --------------------------- | ------------------------------------------------ |
| Screen size                 | 7.5 inch                                         |
| Driver board outline        | 65.0 mm × 30.2 mm                                |
| Display area                | 163.2 mm × 97.92 mm                              |
| Raw panel outline           | 170.2 mm × 111.2 mm × 1.18 mm                    |
| Screen outline              | 177.2 mm × 118.2 mm                              |
| Operating voltage           | 3.3 V / 5 V (IO level must match supply voltage) |
| Communication interface     | SPI                                              |
| Dot pitch                   | 0.205 mm × 0.204 mm                              |
| Resolution                  | **800 × 480**                                    |
| Display colors              | Black, White                                     |
| Grey scale                  | 4 levels                                         |
| Full refresh time           | 4 s                                              |
| Partial refresh time        | 0.4 s                                            |
| Fast refresh time           | 1.5 s                                            |
| Four-grayscale refresh time | 2.1 s                                            |
| Refresh power               | 26.4 mW (typ.)                                   |
| Standby current             | < 0.01 µA (effectively 0)                        |
| Operating temperature       | 0 ~ 50 °C                                        |
| Storage temperature         | −25 ~ 70 °C                                      |

> **Notes:**
> - Refresh times are experimental; actual values may vary.
> - Partial refresh causes no visible flicker; full refresh causes flicker (normal behavior — removes ghosting).
> - Low-temperature refresh may cause color cast; let the display sit at 25 °C for 6 hours before refreshing if this occurs.

---

## Communication Method

SPI (Serial Peripheral Interface).

---

## Operating Principle

E-paper uses electrophoresis: charged black and white particles move under an applied voltage, driven by a waveform sequence (LUT). The waveform is OTP (factory-burned into driver IC) or can use external REGISTER waveforms.

---

## Program Principle — Pixel Encoding

1-bit per pixel:
- `1` (bit high) = **White** □
- `0` (bit low) = **Black** ■

1 byte encodes 8 pixels. Data is stored MSB-first.

**Example:** First 8 pixels black, next 8 white → `0x00 0xFF`

---

## Precautions

1. **Partial refresh limit** — Do not use partial refresh exclusively. Perform a full refresh periodically to prevent irreparable display degradation.
2. **No prolonged power-on** — After each refresh, put the display into sleep mode or power it off. Leaving it in high-voltage state causes permanent damage.
3. **Minimum refresh interval** — At least 180 s between refreshes; refresh at least once every 24 hours. Clear the screen before long-term storage.
4. **Sleep mode** — Image data sent while in sleep mode is ignored. Re-initialize before refreshing after sleep.
5. **Border color** — Controlled via register `0x3C` or `0x50` (Border Waveform Control / VCOM AND DATA INTERVAL SETTING).
6. **Image size check** — If image appears incorrect, verify width/height settings match 800 × 480.
7. **Voltage level** — Working voltage is 3.3 V. Raw panels need a level-conversion circuit for 5 V systems. Driver board V2.1+ supports both 3.3 V and 5 V.
8. **FPC cable** — Do not bend perpendicular to screen, do not repeatedly flex, do not bend toward the front face. Fix cable before use.
9. **Fragile screen** — Avoid drops, impacts, and hard pressure.
10. **Use official sample programs** for initial testing with your development board.

---

## Electrical Characteristics (Quick Reference)

Supply voltage (VCI/VDDIO/VDD): **2.3 – 3.6 V** (typ. 3.3 V).

| Parameter | Min | Typ | Max | Unit |
|-----------|-----|-----|-----|------|
| Logic supply (VCI) | −0.3 | — | +6.0 | V |
| IO supply (VDDIO) | 2.3 | 3.3 | 3.6 | V |
| Image update current | — | 8 | 12 | mA |
| Image update power | — | 26.4 | 45 | mW |
| Standby panel current | — | 0.215 | 0.225 | mA |
| Deep sleep current | — | 2 | 5 | µA |
| Image update time @ 25 °C | — | 4 | 8 | s |
| Internal oscillator | — | 1.625 | — | MHz |

> See [spec/spec.md §3](./spec/spec.md) for full DC/AC characteristics and timing tables.

---

## SPI Interface Pins (24-pin FPC connector)

| Pin | Name | Direction | Description |
|-----|------|-----------|-------------|
| 8 | BS | I | SPI mode: LOW = 4-wire SPI, HIGH = 3-wire SPI (9-bit) |
| 9 | BUSY\_N | O | LOW = driver busy, do not send commands |
| 10 | RST\_N | I | Global reset, active LOW |
| 11 | DC | I | HIGH = data, LOW = command (4-wire SPI only) |
| 12 | CSB | I | Chip select, active LOW |
| 13 | SCL | I | SPI clock |
| 14 | SDA | I/O | SPI data |

**Default mode: 4-wire SPI** (BS tied LOW). Write command: CSB=L, DC=L, rising SCL. Write data: CSB=L, DC=H, rising SCL. Data shifts MSB-first.

> See [spec/spec.md §3.3](./spec/spec.md) for full SPI timing parameters (setup/hold times, clock cycle minimums).

---

## Key Commands (from [spec/spec.md §5](./spec/spec.md))

| Hex | Name | Purpose |
|-----|------|---------|
| `0x00` | Panel Setting (PSR) | LUT source (OTP/register), KW/KWR mode, scan direction |
| `0x01` | Power Setting (PWR) | Enable internal DC/DC, set VGH/VGL/VDH/VDL levels |
| `0x02` | Power OFF (POF) | Turn off booster, drivers, VCOM |
| `0x04` | Power ON (PON) | Turn on booster, regulators; wait for BUSY\_N high |
| `0x07` | Deep Sleep (DSLP) | Low-power sleep; requires check code `0xA5`; wake by RST\_N |
| `0x10` | Data Tx 1 (DTM1) | Write B/W pixel data to SRAM (KW: "OLD" data) |
| `0x11` | Data Stop (DSP) | End data transmission |
| `0x12` | Display Refresh (DRF) | Trigger panel refresh from SRAM |
| `0x13` | Data Tx 2 (DTM2) | Write second buffer (KW: "NEW" data) |
| `0x17` | Auto Sequence (AUTO) | `0xA5` = PON→DRF→POF; `0xA7` = PON→DRF→POF→DSLP |
| `0x30` | PLL Control | Set frame rate (5–200 Hz) |
| `0x50` | VCOM & Data Interval (CDI) | Border color, DDX polarity, VCOM interval |
| `0x61` | Resolution (TRES) | Set HRES/VRES (must match 800×480) |
| `0x82` | VCOM DC Setting (VDCS) | Adjust VCOM voltage (affects contrast/ghosting) |
| `0x90` | Partial Window (PTL) | Define partial refresh region |
| `0x91` | Partial In (PTIN) | Enter partial refresh mode |
| `0x92` | Partial Out (PTOUT) | Exit partial refresh mode |

---

## Typical Operating Sequence

**Full refresh (LUT from OTP):**
1. Reset (RST\_N low → high)
2. Panel Setting `0x00`
3. Power Setting `0x01`
4. Booster Soft Start `0x06`
5. Resolution `0x61` → 800×480
6. VCOM interval `0x50`
7. Power ON `0x04` → wait BUSY\_N high
8. Send image: DTM1 `0x10` + DTM2 `0x13`
9. Display Refresh `0x12` → wait BUSY\_N high
10. Power OFF `0x02` → Deep Sleep `0x07` + `0xA5`

> See [spec/spec.md §4](./spec/spec.md) for full flow diagrams.
