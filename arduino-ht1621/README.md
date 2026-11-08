# HT1621 segment LCD demo (Arduino)

A small driver for the Holtek HT1621 and a sketch that helps you bring up a
new segment LCD:

- **Segment finder** lights one SEG/COM bit at a time and prints its address,
  so you can map a custom glass without guessing.
- **All-on test** for checking contrast and ghosting.
- **7-segment counter** once you have filled in the map for your glass.

The driver itself (`HT1621.h`) has no dependencies beyond the Arduino core and
is covered by a host-side test of the bit frames (`test/ht1621_host_test.cpp`).

## Wiring (Arduino Uno / Nano)

| HT1621 | Arduino |
| --- | --- |
| CS | D10 |
| WR | D11 |
| DATA | D12 |
| VDD / VSS | 5 V or 3.3 V / GND |
| VLCD | contrast: resistor or trimmer to VDD, see the module drawing |

`RD` is not used.

## Setting duty and bias

`LCD_BIAS` must match the LCD drawing. For example, 1/4 duty with 1/3 bias
is `HT1621::BIAS_THIRD_4COM`, and 1/3 duty with 1/2 bias is
`HT1621::BIAS_HALF_3COM`. A mismatch shows up as faint "off" segments
(ghosting) or very low contrast.

More background: [Segment LCD duty and bias](https://successlcd.com/segment-lcd-duty-bias/).

## Mapping a custom glass

1. Upload, open the serial monitor at 115200 and send `f`.
2. Each step lights one bit and prints `SEGx COMy`. Send `p` to pause and
   `n` / `b` to step.
3. Write down which segment lights for each address/COM pair.
4. Fill in `DIGIT_MAP` in `ht1621_demo.ino`, set `MAP_READY` to `1`,
   upload again and send `d`.

## Host test

```sh
g++ -std=c++11 -I test/stub -I arduino-ht1621/ht1621_demo \
    test/ht1621_host_test.cpp -o ht1621_test && ./ht1621_test
```
