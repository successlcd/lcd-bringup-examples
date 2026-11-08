// panel_test.ino
// Bring-up test pattern for SPI TFT panels (ESP32 + LovyanGFX).
//
// Select the panel and pins in lgfx_config.h, flash, and open the serial
// monitor at 115200. Each step tells you what a correct result looks like.
//
// https://github.com/successlcd/lcd-bringup-examples

#include "lgfx_config.h"

static LGFX lcd;

static uint16_t RED, GREEN, BLUE, WHITE, BLACK, YELLOW, GREY;

static void centerText(const char* text, int y, uint16_t fg, uint16_t bg)
{
  lcd.setTextColor(fg, bg);
  int x = (lcd.width() - lcd.textWidth(text)) / 2;
  lcd.drawString(text, x < 0 ? 0 : x, y);
}

static void step(const char* title, const char* hint)
{
  Serial.println();
  Serial.print("== ");
  Serial.println(title);
  Serial.println(hint);
}

// 1. Solid colours: checks colour order and inversion.
static void testColours()
{
  step("Solid colours",
       "You should see RED, GREEN, BLUE, WHITE, BLACK with matching labels.\n"
       "Red and blue swapped -> flip rgb_order. White shows as black -> flip invert.");

  struct { uint16_t c; const char* name; uint16_t text; } list[] = {
    { RED, "RED", WHITE }, { GREEN, "GREEN", BLACK }, { BLUE, "BLUE", WHITE },
    { WHITE, "WHITE", BLACK }, { BLACK, "BLACK", WHITE },
  };
  lcd.setTextSize(2);
  for (auto& s : list) {
    lcd.fillScreen(s.c);
    centerText(s.name, lcd.height() / 2 - 8, s.text, s.c);
    delay(1200);
  }
}

// 2. Border and rotation: checks RAM offsets and mirroring.
static void testBorderAndRotation()
{
  step("Border and rotation",
       "A 1-pixel white border must be visible on all four edges, in every rotation.\n"
       "Missing edge or a strip of noise -> offset_x / offset_y / memory size is wrong.\n"
       "Arrow should point to the top edge; text mirrored -> wrong rotation/MADCTL.");

  for (int r = 0; r < 4; r++) {
    lcd.setRotation(r);
    int w = lcd.width(), h = lcd.height();
    lcd.fillScreen(BLACK);
    lcd.drawRect(0, 0, w, h, WHITE);
    lcd.fillRect(2, 2, 10, 10, RED);            // top-left marker
    lcd.fillTriangle(w / 2, 14, w / 2 - 14, 38, w / 2 + 14, 38, YELLOW);
    char buf[24];
    snprintf(buf, sizeof(buf), "rotation %d", r);
    lcd.setTextSize(1);
    centerText(buf, h / 2 - 4, WHITE, BLACK);
    snprintf(buf, sizeof(buf), "%d x %d", w, h);
    centerText(buf, h / 2 + 10, GREY, BLACK);
    delay(1500);
  }
  lcd.setRotation(0);
}

// 3. Gradients: checks bit depth and banding.
static void testGradients()
{
  step("Gradients",
       "Four smooth ramps (R, G, B, grey) from dark to bright.\n"
       "Hard steps are normal for 16-bit colour (32/64 levels); missing ramps point to wiring.");

  int w = lcd.width(), h = lcd.height();
  int band = h / 4;
  lcd.startWrite();
  for (int x = 0; x < w; x++) {
    uint8_t v = (uint32_t)x * 255 / (w > 1 ? w - 1 : 1);
    lcd.drawFastVLine(x, 0 * band, band, lcd.color565(v, 0, 0));
    lcd.drawFastVLine(x, 1 * band, band, lcd.color565(0, v, 0));
    lcd.drawFastVLine(x, 2 * band, band, lcd.color565(0, 0, v));
    lcd.drawFastVLine(x, 3 * band, h - 3 * band, lcd.color565(v, v, v));
  }
  lcd.endWrite();
  delay(2500);
}

// 4. Full-screen fill speed: rough SPI throughput check.
static void testSpeed()
{
  step("Fill speed", "Prints full-screen fills per second and the effective bus rate.");

  const int frames = 20;
  uint32_t t0 = millis();
  for (int i = 0; i < frames; i++) lcd.fillScreen((i & 1) ? BLACK : GREY);
  uint32_t dt = millis() - t0;
  if (dt == 0) dt = 1;

  float fps = frames * 1000.0f / dt;
  float mbps = (float)lcd.width() * lcd.height() * 16 * frames / (dt * 1000.0f);
  Serial.printf("%d frames in %lu ms -> %.1f fps, ~%.1f Mbit/s of pixel data\n",
                frames, (unsigned long)dt, fps, mbps);
  Serial.println("Theory at 16 bpp: width x height x 16 bits per frame. "
                 "240x240 at 40 MHz is about 23 ms per frame (~43 fps) before overhead.");

  char buf[32];
  snprintf(buf, sizeof(buf), "%.1f fps", fps);
  lcd.fillScreen(BLACK);
  lcd.setTextSize(2);
  centerText(buf, lcd.height() / 2 - 8, WHITE, BLACK);
  delay(1500);
}

// 5. Backlight PWM.
static void testBacklight()
{
  step("Backlight",
       "Brightness should fade down and back up smoothly.\n"
       "Inverted fade -> set invert in the Light_PWM config. No change -> check PIN_BL and driver.");

  lcd.fillScreen(WHITE);
  for (int b = 255; b >= 10; b -= 5) { lcd.setBrightness(b); delay(15); }
  for (int b = 10; b <= 255; b += 5) { lcd.setBrightness(b); delay(15); }
  lcd.setBrightness(200);
}

void setup()
{
  Serial.begin(115200);
  delay(300);

  lcd.init();
  lcd.setBrightness(200);

  RED    = lcd.color565(255, 0, 0);
  GREEN  = lcd.color565(0, 255, 0);
  BLUE   = lcd.color565(0, 0, 255);
  WHITE  = lcd.color565(255, 255, 255);
  BLACK  = lcd.color565(0, 0, 0);
  YELLOW = lcd.color565(255, 220, 0);
  GREY   = lcd.color565(96, 96, 96);

  Serial.println();
  Serial.printf("Panel test: %d x %d (LCD_PANEL=%d)\n", lcd.width(), lcd.height(), LCD_PANEL);
  Serial.println("Blank or white screen with the backlight on? See docs/tft-bringup-checklist.md");
}

void loop()
{
  testColours();
  testBorderAndRotation();
  testGradients();
  testSpeed();
  testBacklight();
}
