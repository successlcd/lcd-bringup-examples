// HT1621.h
// Minimal driver for the Holtek HT1621 segment LCD controller (3-wire: CS, WR, DATA).
//
// Display RAM: 32 addresses (SEG0..SEG31) x 4 bits (COM0..COM3).
// Which address/bit lights which segment depends on the glass, so this
// driver works at address/bit level. Use the segment finder in the demo
// sketch to build the map for your own LCD.
//
// Frame formats (HT1621 datasheet):
//   command: 1 0 0  C7..C0  X            (12 bits, MSB first, X = don't care)
//   write:   1 0 1  A5..A0  D0 D1 D2 D3  (address MSB first, data LSB first)
// Data is latched on the rising edge of WR. Consecutive nibbles after one
// address auto-increment the address.

#pragma once

#include <Arduino.h>
#include <string.h>

class HT1621
{
public:
  // Command codes
  static const uint8_t SYS_DIS         = 0x00;
  static const uint8_t SYS_EN          = 0x01;
  static const uint8_t LCD_OFF         = 0x02;
  static const uint8_t LCD_ON          = 0x03;
  static const uint8_t TIMER_DIS       = 0x04;
  static const uint8_t WDT_DIS         = 0x05;
  static const uint8_t TONE_OFF        = 0x08;
  static const uint8_t XTAL_32K        = 0x14;
  static const uint8_t RC_256K         = 0x18;  // internal RC oscillator
  static const uint8_t BIAS_HALF_2COM  = 0x20;
  static const uint8_t BIAS_HALF_3COM  = 0x24;
  static const uint8_t BIAS_HALF_4COM  = 0x28;
  static const uint8_t BIAS_THIRD_2COM = 0x21;
  static const uint8_t BIAS_THIRD_3COM = 0x25;
  static const uint8_t BIAS_THIRD_4COM = 0x29;
  static const uint8_t IRQ_DIS         = 0x80;

  static const uint8_t ADDR_COUNT = 32;

  HT1621(uint8_t csPin, uint8_t wrPin, uint8_t dataPin)
    : cs_(csPin), wr_(wrPin), data_(dataPin)
  {
    memset(ram_, 0, sizeof(ram_));
  }

  // biasCmd must match your glass: duty (number of COMs) and bias are on the LCD drawing.
  void begin(uint8_t biasCmd = BIAS_THIRD_4COM)
  {
    pinMode(cs_, OUTPUT);
    pinMode(wr_, OUTPUT);
    pinMode(data_, OUTPUT);
    digitalWrite(cs_, HIGH);
    digitalWrite(wr_, HIGH);
    digitalWrite(data_, HIGH);

    command(SYS_EN);
    command(RC_256K);
    command(biasCmd);
    command(TIMER_DIS);
    command(WDT_DIS);
    command(TONE_OFF);
    command(IRQ_DIS);
    clear();
    command(LCD_ON);
  }

  void command(uint8_t cmd)
  {
    select();
    sendMsbFirst(0x4, 3);    // 100
    sendMsbFirst(cmd, 8);
    sendMsbFirst(0, 1);      // don't-care bit
    deselect();
  }

  // Write one 4-bit value (COM0 = bit 0 ... COM3 = bit 3) to an address.
  void writeNibble(uint8_t addr, uint8_t nibble)
  {
    ram_[addr & 0x1F] = nibble & 0x0F;
    writeRaw(addr, &ram_[addr & 0x1F], 1);
  }

  // Shadow-RAM helpers: change bits, then flush() once.
  void setBit(uint8_t addr, uint8_t com, bool on)
  {
    addr &= 0x1F;
    com &= 0x03;
    if (on) ram_[addr] |= (1 << com);
    else    ram_[addr] &= ~(1 << com);
  }

  bool getBit(uint8_t addr, uint8_t com) const
  {
    return ram_[addr & 0x1F] & (1 << (com & 0x03));
  }

  void fill(bool on)
  {
    memset(ram_, on ? 0x0F : 0x00, sizeof(ram_));
  }

  void flush()
  {
    writeRaw(0, ram_, ADDR_COUNT);
  }

  void clear()
  {
    fill(false);
    flush();
  }

  // Half period of WR. 4 us -> 125 kHz, inside the HT1621 limit at 3 V and 5 V.
  static const uint8_t HALF_PERIOD_US = 4;

private:
  void writeRaw(uint8_t addr, const uint8_t* nibbles, uint8_t count)
  {
    select();
    sendMsbFirst(0x5, 3);            // 101
    sendMsbFirst(addr & 0x3F, 6);    // A5..A0
    for (uint8_t i = 0; i < count; i++) {
      sendLsbFirst(nibbles[i] & 0x0F, 4);  // D0..D3
    }
    deselect();
  }

  void select()   { digitalWrite(cs_, LOW); }
  void deselect() { digitalWrite(cs_, HIGH); digitalWrite(wr_, HIGH); }

  void clockBit(bool bit)
  {
    digitalWrite(wr_, LOW);
    digitalWrite(data_, bit ? HIGH : LOW);
    delayMicroseconds(HALF_PERIOD_US);
    digitalWrite(wr_, HIGH);         // HT1621 latches DATA here
    delayMicroseconds(HALF_PERIOD_US);
  }

  void sendMsbFirst(uint16_t value, uint8_t bits)
  {
    for (int8_t i = bits - 1; i >= 0; i--) clockBit((value >> i) & 1);
  }

  void sendLsbFirst(uint16_t value, uint8_t bits)
  {
    for (uint8_t i = 0; i < bits; i++) clockBit((value >> i) & 1);
  }

  uint8_t cs_, wr_, data_;
  uint8_t ram_[ADDR_COUNT];
};
