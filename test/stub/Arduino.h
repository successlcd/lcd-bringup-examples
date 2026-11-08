// Minimal Arduino API stub for host-side tests of HT1621.h.
// Records the DATA level on every rising edge of WR while CS is low.
#pragma once

#include <stdint.h>
#include <vector>

#define HIGH 1
#define LOW 0
#define OUTPUT 1

namespace stub {
struct Bus {
  int cs = -1, wr = -1, data = -1;
  int csLevel = 1, wrLevel = 1, dataLevel = 1;
  std::vector<std::vector<int>> frames;   // one vector of bits per CS-low period
};
inline Bus& bus() { static Bus b; return b; }
}

inline void pinMode(uint8_t, uint8_t) {}
inline void delayMicroseconds(unsigned int) {}

inline void digitalWrite(uint8_t pin, uint8_t level)
{
  stub::Bus& b = stub::bus();
  if (pin == b.cs) {
    if (b.csLevel == 1 && level == 0) b.frames.push_back({});
    b.csLevel = level;
  } else if (pin == b.wr) {
    if (b.wrLevel == 0 && level == 1 && b.csLevel == 0 && !b.frames.empty())
      b.frames.back().push_back(b.dataLevel);
    b.wrLevel = level;
  } else if (pin == b.data) {
    b.dataLevel = level;
  }
}
