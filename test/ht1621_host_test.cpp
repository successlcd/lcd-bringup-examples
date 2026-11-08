// Host-side test for HT1621.h: checks the exact bit frames against the
// HT1621 datasheet formats. Build and run:
//   g++ -std=c++11 -I test/stub -I arduino-ht1621/ht1621_demo test/ht1621_host_test.cpp -o ht1621_test && ./ht1621_test

#include "HT1621.h"
#include <cstdio>
#include <string>

static int failures = 0;

static std::string bits(const std::vector<int>& v)
{
  std::string s;
  for (int b : v) s += b ? '1' : '0';
  return s;
}

static void expect(const char* name, const std::vector<int>& got, const std::string& want)
{
  std::string g = bits(got);
  if (g != want) {
    std::printf("FAIL %s\n  got  %s\n  want %s\n", name, g.c_str(), want.c_str());
    failures++;
  } else {
    std::printf("ok   %s\n", name);
  }
}

int main()
{
  stub::Bus& b = stub::bus();
  b.cs = 10; b.wr = 11; b.data = 12;

  HT1621 lcd(10, 11, 12);

  // begin(): 7 commands, one full RAM write (clear), then LCD_ON
  lcd.begin(HT1621::BIAS_THIRD_4COM);
  if (b.frames.size() != 9) { std::printf("FAIL begin frame count %zu\n", b.frames.size()); failures++; }
  else std::printf("ok   begin sends 9 frames\n");
  expect("SYS_EN",          b.frames[0], "100" "00000001" "0");
  expect("RC_256K",         b.frames[1], "100" "00011000" "0");
  expect("BIAS 1/3, 4 COM", b.frames[2], "100" "00101001" "0");
  expect("clear",           b.frames[7], "101" "000000" + std::string(32 * 4, '0'));
  expect("LCD_ON",          b.frames[8], "100" "00000011" "0");

  // Single nibble: address MSB first, data D0..D3 (LSB first)
  b.frames.clear();
  lcd.writeNibble(5, 0xA);              // 1010b -> D0=0 D1=1 D2=0 D3=1
  expect("write addr 5 = 0xA", b.frames[0], "101" "000101" "0101");

  // Shadow RAM + flush
  b.frames.clear();
  lcd.fill(false);
  lcd.setBit(3, 2, true);               // address 3, COM2
  lcd.setBit(31, 0, true);              // address 31, COM0
  lcd.flush();
  std::string want = "101" "000000";
  for (int a = 0; a < 32; a++) {
    if (a == 3)       want += "0010";
    else if (a == 31) want += "1000";
    else              want += "0000";
  }
  expect("flush with two bits set", b.frames[0], want);

  if (!lcd.getBit(3, 2) || lcd.getBit(3, 1)) { std::printf("FAIL getBit\n"); failures++; }
  else std::printf("ok   getBit\n");

  std::printf(failures ? "\n%d FAILED\n" : "\nall passed\n", failures);
  return failures ? 1 : 0;
}
