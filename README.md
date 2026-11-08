# LCD bring-up examples

Practical starting points for getting a new display running on a
microcontroller, plus the checklist we go through when a panel misbehaves.

| Folder | What it is |
| --- | --- |
| [`esp32-lovyangfx/`](esp32-lovyangfx/) | ESP32 test pattern for SPI TFTs: ST7789, ST7735S, ILI9341, GC9A01 (round). Checks colour order, inversion, RAM offsets, rotation, speed and backlight. |
| [`arduino-ht1621/`](arduino-ht1621/) | HT1621 segment LCD driver with a segment finder for mapping custom glass. |
| [`docs/tft-bringup-checklist.md`](docs/tft-bringup-checklist.md) | Symptom → cause → fix: white screen, inverted colours, shifted image, noise, slow refresh. |
| [`test/`](test/) | Host-side test for the HT1621 bit frames. |

## Quick start

**SPI TFT on ESP32:** install the esp32 board package and the LovyanGFX
library, choose your panel in
[`lgfx_config.h`](esp32-lovyangfx/panel_test/lgfx_config.h), upload
`panel_test`, and follow the serial monitor.

**Segment LCD with HT1621:** wire CS/WR/DATA to D10/D11/D12, set duty and bias
in `ht1621_demo.ino`, upload, and send `f` to start the segment finder.

## A note on init sequences

The examples use the generic init sequences that ship with each library. They
bring up most glass, but two modules with the same driver IC can still need
different power, gamma or offset settings. The datasheet for the specific
module is the reference. If you are working with one of our modules and need
its exact init code, ask us.

## Related

- [lcd-display-calculators](https://github.com/successlcd/lcd-display-calculators): pixel clock, interface bandwidth and frame-buffer calculators
- [segment-lcd-references](https://github.com/successlcd/segment-lcd-references): 428 segment LCD drawings in one searchable table
- Application notes at [successlcd.com/engineering-articles](https://successlcd.com/engineering-articles/)

Issues and pull requests are welcome, especially configs for panels that
need non-default offsets or settings.

## About

Maintained by [SuccessLCD](https://successlcd.com/), a TFT, monochrome and
custom segment LCD module supplier for OEM/ODM products.
Contact: sales@successlcd.com

MIT licensed. See [LICENSE](LICENSE).
