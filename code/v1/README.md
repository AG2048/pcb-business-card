# V1 Code

This directory contains the code for the AG Business Card V1. This code is to be run on the ATTiny84 microcontroller on the PCB. It reads the ST25DV64K NFC tag for display patterns and displays them on the 6x12 RGB LED matrix.

## Requirements
- ISP Programmer (Used Arduino Uno as ISP)
- [ATTinyCore](https://github.com/SpenceKonde/ATTinyCore)
  - [Wire](https://github.com/SpenceKonde/ATTinyCore/tree/v2.0.0-devThis-is-the-head-submit-PRs-against-this/avr/libraries/Wire) included with ATTinyCore, this allows the use of the hardware I2C on the ATTiny84.
  - [tinyNeoPixel_Static](https://github.com/SpenceKonde/ATTinyCore/tree/v2.0.0-devThis-is-the-head-submit-PRs-against-this/avr/libraries/tinyNeoPixel_Static) for controlling the RGB LED matrix. The static version is used

## Programming
The pinout for ATTiny84 can be found in the [ATTinyCore documentation](https://github.com/SpenceKonde/ATTinyCore/blob/v2.0.0-devThis-is-the-head-submit-PRs-against-this/avr/extras/ATtiny_x4.md).