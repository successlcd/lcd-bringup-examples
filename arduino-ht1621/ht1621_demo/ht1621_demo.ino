// ht1621_demo.ino
// HT1621 segment LCD bring-up: segment finder, all-on test and a 7-segment counter.
//
// Serial monitor at 115200, then send:
//   f  segment finder: lights one SEG/COM bit at a time and prints its address
//   p  pause / resume the finder;  n / b  next / previous bit while paused
//   a  all segments on (contrast and ghosting check)
//   c  clear
//   d  digit counter (after you fill in DIGIT_MAP below)
//
// https://github.com/successlcd/lcd-bringup-examples

#include "HT1621.h"

// Wiring (Arduino Uno / Nano defaults)
const uint8_t PIN_CS   = 10;
const uint8_t PIN_WR   = 11;
const uint8_t PIN_DATA = 12;

// Duty and bias must match the LCD drawing, e.g. 1/4 duty 1/3 bias -> BIAS_THIRD_4COM.
const uint8_t LCD_BIAS = HT1621::BIAS_THIRD_4COM;

HT1621 lcd(PIN_CS, PIN_WR, PIN_DATA);

// ---------------------------------------------------------------------------
// 7-segment map for YOUR glass. Run the finder ('f'), note which address/COM
// lights each segment, fill this table in, then set MAP_READY to 1.
// The values below are placeholders, not a real layout.
// ---------------------------------------------------------------------------
#define MAP_READY 0

struct SegPos { uint8_t addr; uint8_t com; };

const uint8_t NUM_DIGITS = 2;
//                                  a        b        c        d        e        f        g
const SegPos DIGIT_MAP[NUM_DIGITS][7] = {
  { {0, 3}, {0, 2}, {0, 1}, {1, 0}, {1, 1}, {1, 3}, {1, 2} },   // digit 0 (leftmost)
  { {2, 3}, {2, 2}, {2, 1}, {3, 0}, {3, 1}, {3, 3}, {3, 2} },   // digit 1
};

// Bit 0 = segment a ... bit 6 = segment g
const uint8_t FONT[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };

enum Mode { MODE_IDLE, MODE_FINDER, MODE_COUNTER };
Mode mode = MODE_IDLE;

bool    paused = false;
uint8_t findIndex = 0;            // 0..127 = addr * 4 + com
unsigned long lastStep = 0;
uint16_t counter = 0;

void showFinderBit()
{
  uint8_t addr = findIndex / 4;
  uint8_t com  = findIndex % 4;
  lcd.fill(false);
  lcd.setBit(addr, com, true);
  lcd.flush();
  Serial.print("SEG");
  Serial.print(addr);
  Serial.print(" COM");
  Serial.print(com);
  Serial.print("   (address ");
  Serial.print(addr);
  Serial.print(", bit ");
  Serial.print(com);
  Serial.println(")");
}

void showNumber(uint16_t value)
{
  lcd.fill(false);
  for (int8_t i = NUM_DIGITS - 1; i >= 0; i--) {
    uint8_t pattern = FONT[value % 10];
    value /= 10;
    for (uint8_t s = 0; s < 7; s++) {
      if (pattern & (1 << s)) lcd.setBit(DIGIT_MAP[i][s].addr, DIGIT_MAP[i][s].com, true);
    }
  }
  lcd.flush();
}

void printHelp()
{
  Serial.println(F("Commands: f=finder  p=pause  n/b=next/prev  a=all on  c=clear  d=counter"));
}

void setup()
{
  Serial.begin(115200);
  lcd.begin(LCD_BIAS);
  Serial.println(F("HT1621 bring-up demo"));
  Serial.println(F("Nothing at all on 'a'? Check VDD, the VLCD (contrast) pin and the bias setting."));
  Serial.println(F("Faint 'off' segments (ghosting)? Bias/duty mismatch or VLCD too high."));
  printHelp();
}

void loop()
{
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'f': mode = MODE_FINDER; paused = false; findIndex = 0; showFinderBit(); lastStep = millis(); break;
      case 'p': paused = !paused; Serial.println(paused ? F("paused") : F("running")); break;
      case 'n': if (mode == MODE_FINDER) { findIndex = (findIndex + 1) % 128; showFinderBit(); } break;
      case 'b': if (mode == MODE_FINDER) { findIndex = (findIndex + 127) % 128; showFinderBit(); } break;
      case 'a': mode = MODE_IDLE; lcd.fill(true); lcd.flush(); Serial.println(F("all on")); break;
      case 'c': mode = MODE_IDLE; lcd.clear(); Serial.println(F("cleared")); break;
      case 'd':
#if MAP_READY
        mode = MODE_COUNTER; counter = 0;
#else
        Serial.println(F("Fill in DIGIT_MAP for your glass first, then set MAP_READY to 1."));
#endif
        break;
      case '\r': case '\n': break;
      default: printHelp(); break;
    }
  }

  if (mode == MODE_FINDER && !paused && millis() - lastStep > 800) {
    lastStep = millis();
    findIndex = (findIndex + 1) % 128;
    showFinderBit();
  }

  if (mode == MODE_COUNTER && millis() - lastStep > 500) {
    lastStep = millis();
    showNumber(counter++);
  }
}
