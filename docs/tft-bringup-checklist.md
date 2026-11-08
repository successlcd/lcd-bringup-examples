# TFT bring-up checklist

What to check, in order, when a new SPI TFT does not show what you expect.
Run `esp32-lovyangfx/panel_test` alongside this list: each test step maps
to one of the symptoms below.

## 1. Backlight on, screen stays white (or grey)

The panel is powered but the controller has not been initialised.

- **Wiring:** SCLK, MOSI (SDA/SDI), CS, DC (RS/A0) and RST. Swapping DC and CS is
  common and gives exactly this symptom.
- **Reset:** hold RST low for at least 10 µs, then give the controller time
  before talking to it (ST7789: 5 ms before commands, 120 ms before Sleep Out).
  A floating RST pin can leave the controller in reset.
- **Interface mode:** many modules set SPI vs. parallel/RGB with IM pins on the
  FPC or a solder jumper. If the module is strapped for RGB, SPI commands do nothing.
- **3-wire vs. 4-wire SPI:** 3-wire (9-bit) SPI has no DC pin and sends the D/C
  flag as an extra bit. Libraries default to 4-wire with a DC pin.
- **Driver IC:** confirm the controller from the module datasheet, not the shop
  listing. ST7789, ILI9341 and GC9A01 share many commands but need different
  power and gamma setup.
- **Supply:** logic at 3.3 V. Some modules have a regulator and accept 5 V on
  VCC; many do not.

## 2. Screen stays black

- **Backlight:** LED+/LED− (or BLA/BLK) need the right current. A backlight is
  several LEDs in series or parallel. Check the forward voltage and current
  in the datasheet, and drive it through a resistor, transistor or LED driver,
  never straight from a GPIO.
- **PWM polarity:** with an active-low driver, `setBrightness(255)` turns it off.

## 3. Colours look wrong

| Symptom | Cause | Fix |
| --- | --- | --- |
| White shows as black, everything looks like a negative | Inversion setting | Flip `invert` (INVON/INVOFF). Most IPS ST7789 and GC9A01 glass needs it **on**. |
| Red and blue swapped | RGB/BGR order | Flip `rgb_order` (MADCTL BGR bit). |
| Washed out or too dark at some angles | Viewing direction (TN glass) | Check the 6 o'clock / 12 o'clock viewing direction on the drawing; IPS avoids this. |
| Strong banding in gradients | 16-bit colour (RGB565) | Normal: 32 levels for red and blue, 64 for green. Use 18/24-bit modes if the interface supports them. |

## 4. Image shifted, cut off, or a strip of noise on one edge

The controller RAM is larger than the glass, and the visible window starts at
an offset.

- ST7789 has 240 x 320 RAM. A 240 x 240 panel uses part of it, so set the
  panel to 240 x 240 but keep the memory size at 240 x 320.
- ST7735S has 132 x 162 RAM. 1.44" 128 x 128 glass typically needs
  `offset_x = 2`, `offset_y = 1` (or 3).
- If it is right in one rotation and wrong in another, the memory size is
  wrong, not just the offset.

## 5. Mirrored or upside-down image

MADCTL MX/MY/MV bits. In LovyanGFX use `setRotation()` (0–3, or 4–7 for
mirrored) and `offset_rotation` in the panel config.

## 6. Noise, random pixels, works only sometimes

- **SPI clock:** 40 MHz works on short connections. With jumper wires or a long
  FPC extension, try 27 or 20 MHz first.
- **Signal quality:** a 22–33 Ω series resistor at the driver end of SCLK
  helps with ringing. Keep a ground return next to SCLK.
- **Power:** a backlight switching on can dip a weak 3.3 V rail and reset the
  controller. Add bulk capacitance near the module.
- **ESD:** if it fails after handling, look at the FPC connector and grounding.

## 7. Too slow

A full frame over SPI is `width × height × bits per pixel` bits:

| Panel | Bits per frame | Time at 40 MHz | Max fps (theory) |
| --- | --- | --- | --- |
| 240 x 240, 16 bpp | 921,600 | 23 ms | ~43 |
| 240 x 320, 16 bpp | 1,228,800 | 31 ms | ~32 |
| 320 x 480, 18 bpp sent as 24 bits (ILI9488 SPI) | 3,686,400 | 92 ms | ~11 |

Real numbers are lower. Redraw only what changes, use DMA, and consider a
parallel MCU bus or RGB interface for larger panels. The
[lcd-display-calculators](https://github.com/successlcd/lcd-display-calculators)
repo has bandwidth and frame-buffer calculators.

## 8. Tearing during animation

The panel refreshes while you are writing. Use the TE (tearing effect) output
to time frame writes, or redraw small regions only.

---

Background reading from our application notes:

- [ILI9341 vs ST7789 vs ILI9488](https://successlcd.com/ili9341-vs-st7789-vs-ili9488/)
- [LCD interfaces: I2C, SPI, UART, parallel](https://successlcd.com/lcd-interface-i2c-spi-uart-parallel/)
- [LCD backlight LED guide](https://successlcd.com/lcd-backlight-led-guide/)
- [LCD viewing direction](https://successlcd.com/lcd-viewing-direction/)
