// lgfx_config.h
// LovyanGFX setup for common SPI TFT controllers on ESP32.
//
// Pick ONE panel with LCD_PANEL below, or pass it from the build,
// e.g. arduino-cli ... --build-property "compiler.cpp.extra_flags=-DLCD_PANEL=4"
//
// These are starting points for common glass. The exact init sequence,
// gamma and power settings of a specific module come from its datasheet.
// Two modules with the same driver IC are not always interchangeable.

#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#define PANEL_ST7789_240X240   1  // 1.3" / 1.54" square IPS
#define PANEL_ST7789_240X320   2  // 2.0" / 2.4" / 2.8"
#define PANEL_ILI9341_240X320  3  // 2.4" / 2.8" / 3.2"
#define PANEL_GC9A01_240X240   4  // 1.28" round IPS
#define PANEL_ST7735S_128X128  5  // 1.44"

#ifndef LCD_PANEL
#define LCD_PANEL PANEL_ST7789_240X240
#endif

// ---------------------------------------------------------------------------
// Wiring. Defaults suit a classic ESP32 DevKit; on ESP32-S3/C3 any free GPIO
// works too (the SPI signals go through the GPIO matrix).
// ---------------------------------------------------------------------------
#ifndef PIN_SCLK
#define PIN_SCLK 18
#endif
#ifndef PIN_MOSI
#define PIN_MOSI 23
#endif
#ifndef PIN_MISO
#define PIN_MISO -1   // not needed: these examples never read back from the panel
#endif
#ifndef PIN_DC
#define PIN_DC 16     // also labelled RS or A0 on some FPCs
#endif
#ifndef PIN_CS
#define PIN_CS 5
#endif
#ifndef PIN_RST
#define PIN_RST 17
#endif
#ifndef PIN_BL
#define PIN_BL 4      // drive the backlight through a transistor or LED driver, not directly
#endif

class LGFX : public lgfx::LGFX_Device
{
#if (LCD_PANEL == PANEL_ST7789_240X240) || (LCD_PANEL == PANEL_ST7789_240X320)
  lgfx::Panel_ST7789 _panel;
#elif LCD_PANEL == PANEL_ILI9341_240X320
  lgfx::Panel_ILI9341 _panel;
#elif LCD_PANEL == PANEL_GC9A01_240X240
  lgfx::Panel_GC9A01 _panel;
#elif LCD_PANEL == PANEL_ST7735S_128X128
  lgfx::Panel_ST7735S _panel;
#else
#error "Unknown LCD_PANEL value"
#endif
  lgfx::Bus_SPI   _bus;
  lgfx::Light_PWM _light;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus.config();
      cfg.spi_host   = SPI2_HOST;   // available on ESP32, -S2, -S3, -C3, -C6
      cfg.spi_mode   = 0;
#if LCD_PANEL == PANEL_ST7735S_128X128
      cfg.freq_write = 27000000;    // ST7735S is happier a bit slower
#else
      cfg.freq_write = 40000000;    // above some datasheet limits; fine on short wires.
#endif                              // Drop to 27 or 20 MHz if you see noise.
      cfg.freq_read  = 16000000;
      cfg.spi_3wire  = true;        // single data line, no MISO
      cfg.use_lock   = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = PIN_SCLK;
      cfg.pin_mosi = PIN_MOSI;
      cfg.pin_miso = PIN_MISO;
      cfg.pin_dc   = PIN_DC;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    {
      auto cfg = _panel.config();
      cfg.pin_cs     = PIN_CS;
      cfg.pin_rst    = PIN_RST;
      cfg.pin_busy   = -1;
      cfg.readable   = false;
      cfg.bus_shared = false;
      cfg.rgb_order  = false;       // set true if red and blue come out swapped

#if LCD_PANEL == PANEL_ST7789_240X240
      cfg.panel_width   = 240;
      cfg.panel_height  = 240;
      cfg.memory_width  = 240;      // the controller RAM is 240 x 320; the glass
      cfg.memory_height = 320;      // uses part of it, so keep the real RAM size here
      cfg.invert        = true;     // most ST7789 IPS glass needs inversion ON
#elif LCD_PANEL == PANEL_ST7789_240X320
      cfg.panel_width   = 240;
      cfg.panel_height  = 320;
      cfg.memory_width  = 240;
      cfg.memory_height = 320;
      cfg.invert        = true;     // IPS: usually true. TN glass: usually false
#elif LCD_PANEL == PANEL_ILI9341_240X320
      cfg.panel_width   = 240;
      cfg.panel_height  = 320;
      cfg.memory_width  = 240;
      cfg.memory_height = 320;
      cfg.invert        = false;
#elif LCD_PANEL == PANEL_GC9A01_240X240
      cfg.panel_width   = 240;
      cfg.panel_height  = 240;
      cfg.memory_width  = 240;
      cfg.memory_height = 240;
      cfg.invert        = true;
#elif LCD_PANEL == PANEL_ST7735S_128X128
      cfg.panel_width   = 128;
      cfg.panel_height  = 128;
      cfg.memory_width  = 132;      // 1.44" glass usually sits inside 132 x 132 RAM
      cfg.memory_height = 132;
      cfg.offset_x      = 2;        // typical; if one edge shows a line of noise,
      cfg.offset_y      = 1;        // try offset_y = 3 or memory_height = 162
      cfg.invert        = false;
#endif
      _panel.config(cfg);
    }

    {
      auto cfg = _light.config();
      cfg.pin_bl      = PIN_BL;
      cfg.invert      = false;      // true if your backlight driver is active-low
      cfg.freq        = 20000;      // above the audible range
      cfg.pwm_channel = 7;
      _light.config(cfg);
      _panel.setLight(&_light);
    }

    setPanel(&_panel);
  }
};
