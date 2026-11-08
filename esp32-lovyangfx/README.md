# ESP32 panel test (LovyanGFX)

A single sketch that walks a new SPI TFT through five checks: solid colours,
border and rotation, gradients, fill speed and backlight PWM. The serial
monitor explains what a correct result looks like at each step.

## Supported starting configs

Set `LCD_PANEL` in `panel_test/lgfx_config.h`:

| `LCD_PANEL` | Controller | Resolution | Typical glass |
| --- | --- | --- | --- |
| `PANEL_ST7789_240X240` | ST7789 | 240 x 240 | 1.3" / 1.54" square IPS |
| `PANEL_ST7789_240X320` | ST7789 | 240 x 320 | 2.0" – 2.8" |
| `PANEL_ILI9341_240X320` | ILI9341 | 240 x 320 | 2.4" – 3.2" |
| `PANEL_GC9A01_240X240` | GC9A01 | 240 x 240 | 1.28" round IPS |
| `PANEL_ST7735S_128X128` | ST7735S | 128 x 128 | 1.44" |

These use the library's built-in init sequences. They get most glass working,
but a specific module may need its own power or gamma settings from the
datasheet.

## Wiring (defaults)

| Panel pin | Also labelled | ESP32 GPIO |
| --- | --- | --- |
| SCL | SCLK, SCK, CLK | 18 |
| SDA | MOSI, SDI, DIN | 23 |
| DC | RS, A0 | 16 |
| CS | | 5 |
| RST | RES, RESET | 17 |
| BL | LED, BLK (through a driver) | 4 |
| VCC / GND | | 3.3 V / GND |

Change the pins with the `PIN_*` defines. On ESP32-S3 and ESP32-C3 any free
GPIO works.

## Build

Arduino IDE 2.x:

1. Boards Manager: install **esp32** by Espressif.
2. Library Manager: install **LovyanGFX**.
3. Open `panel_test/panel_test.ino`, pick your board, upload.

arduino-cli:

```sh
arduino-cli lib install LovyanGFX
arduino-cli compile --fqbn esp32:esp32:esp32 esp32-lovyangfx/panel_test \
  --build-property "compiler.cpp.extra_flags=-DLCD_PANEL=4"
```

If something looks off, the [bring-up checklist](../docs/tft-bringup-checklist.md)
goes through the usual causes.
