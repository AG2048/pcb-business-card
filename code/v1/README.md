# V1 Code

This directory contains the code for the AG Business Card V1. This code is to be run on the ATTiny84 microcontroller on the PCB. It reads the ST25DV64K NFC tag for display patterns and displays them on the 6x12 RGB LED matrix.

## Requirements
- ISP Programmer (Used Arduino Uno as ISP)
- [ATTinyCore](https://github.com/SpenceKonde/ATTinyCore)
  - [Wire](https://github.com/SpenceKonde/ATTinyCore/tree/v2.0.0-devThis-is-the-head-submit-PRs-against-this/avr/libraries/Wire) included with ATTinyCore, this allows the use of the hardware I2C on the ATTiny84.
  - [tinyNeoPixel_Static](https://github.com/SpenceKonde/ATTinyCore/tree/v2.0.0-devThis-is-the-head-submit-PRs-against-this/avr/libraries/tinyNeoPixel_Static) for controlling the RGB LED matrix. The static version is used

## Pins
The pinout for ATTiny84 can be found in the [ATTinyCore documentation](https://github.com/SpenceKonde/ATTinyCore/blob/v2.0.0-devThis-is-the-head-submit-PRs-against-this/avr/extras/ATtiny_x4.md). 

The most relevant pin is the LED pin, defined as `LED_PIN` in the code, which is connected to pin 10 of the ATTiny84. This pin is used to control the RGB LED matrix. 

When using the `tinyNeoPixel_Static` library, I configured the bootloader to use PORT B (which is where pin 10 is located). 

The I2C pins uses the standard SDA and SCL pins, so `Wire.begin()` will automatically use the correct pins.

## LED Matrix
The `tinyNeoPixel_Static` library is used to control the RGB LED matrix. This library uses a static array of bytes to store the pixel data in `GRB888` format. The pixels can be modified by setting corresponding values in the `pixel_data` array.

After some preliminary tests, the brightness of each colour of the RGB LEDs is set to be capped at 15 (0xF) to prevent:
1. Excess power draw -- as a "full white" test of max brightness (255) caused the LED matrix to appear red / orange (not enough power supply to light up the green and blue LEDs sufficiently).
2. Excess brightness -- at 255, the LEDs' brightness can be "blinding", and regular phone cameras would likely just capture a "white blur" instead of individual pixels. 

This setup can also save memory. Since a `GRB888` array for 72 pixels takes up 216 bytes out of the total 512 bytes RAM on ATTiny84, and if the brightness is capped at 15 (only 4 bits per channel), the memory requirement can effectively be halved. 

### tinyNeoPixel_Static Modifications
To accomodate a reduced-sized pixels array, the original `tinyNeoPixel_Static` library can no longer be used. 
1. The array now stores GRBGRBGRBGRB... tightly in GRB444 format (Meaning two neighbouring pixels may share bytes)
2. The `show()` function's assembly code is modified to default display `0` for the higher 4 bits. 
   - Modify higher 4 bits with an extra `"st  %a[port] , %[lo]"    "\n\t"`

A custom leds_set_pixel_color function is used to set the pixel color in the `pixel_data` array. This function takes the x and y coordinates of the pixel, and the red, green, and blue values to set the pixel color. This is used because the LEDs are arranged in a zig-zag pattern, so the pixel data needs to be set in a specific order.

## NFC I2C
The ST25DV64K NFC tag is used to store the display patterns for the RGB LED matrix. The NFC tag is accessed using the I2C protocol, and the `Wire` library is used to communicate with the NFC tag.

To read the NFC tag, the `nfc_read_data` function is used. This function reads a specified number of bytes from the NFC tag starting at a specified address. The data is stored in the `nfc_data` buffer array.

Inside `nfc_read_data`, we first use `Wire.beginTransmission(NFC_I2C_ADDRESS)` to begin a `WRITE` operation to the NFC tag, where we write the 2-byte address we wish to read from (upper byte first). According to the ST25DV64K datasheet, we need to send a `RESTART` condition and switch to a `READ` operation to read the data from memory. This was done using `Wire.endTransmission(false)` to keep the I2C bus open, and then using `Wire.requestFrom(NFC_I2C_ADDRESS, length)` to read the data from the NFC tag.

`NFC_I2C_ADDRESS` is defined as `0b1010011`, which is the NFC tag's address that accesses the user memory. 

## Pattern Encoding
**NOTE THIS IS A WORK IN PROGRESS**

The NFC tag is supposed to store [andygong.com](https://andygong.com) as its URL readable by tapping the card on a phone. This means that any graphical patterns should start in **byte 32** of the NFC tag memory. When writing to the NFC tag, this is data **block 8**. 

72 pixels of 1.5 bytes each (in `GRB444` format) uses 108 bytes for pixel data. Meaning that out of 8 kB of user memory, we can store 75 frames as a maximum. 

Each frame should have some metadata:
- Colour mode -- as in if each bit of data represent a single pixel, or the data is in `GRB444` format.
  - Single pixel mode only requires 1 bit per pixel, so 9 bytes for 72 pixels. (A default colour value can be set for the entire frame, such as red, green, blue, orange, yellow... etc.)
  - `GRB444` format requires 1.5 bytes per pixel, so 108 bytes for 72 pixels.
  - This can be set by using 3 bits (`000` for `GRB444` mode, and the rest are for `GRB` combinations for single pixel mode, such as `001` is for blue, `110` is for green+red=yellow, etc.)
- Transition format
  - If the frame is instantly changed from previous frame
  - If the frame "fades" from previous frame
  - This will be set using 13 bits, representing the transition time in milliseconds. (This can be set to 0 for instant change, or a value between 1 and 8191 milliseconds for a fade transition.)
- Frame duration -- how long the frame should be displayed for, in milliseconds.
  - This will use 2 bytes (16 bits) for up to 65,535 milliseconds (or 65.535 seconds) of display time.

Overall, each frame will have 4 bytes of metadata (`CCCT_TTTT_TTTT_TTTT_DDDD_DDDD_DDDD_DDDD`), where:
- `CCC` is the colour mode (3 bits)
- `TTT_TTTT_TTTT` is the transition time (13 bits)
- `DDDD_DDDD_DDDD_DDDD` is the frame duration (16 bits)

This aligns with the 32-bit word size of the NFC tag.

With the metadata, each frame will have a minimum of `4+9=13` bytes for single pixel mode, or `4+108=112` bytes for `GRB444` mode. So we can fit a minimum of 72 frames to a maximum of 627 frames. 

The first few bytes should store some data about the overall storage. Such as:
- The number of images/frames stored in the NFC tag -- so that the code knows when to stop reading. 
  - This should just use 2 bytes as maximum is 627 > 255 for 1 byte storage. 
- The remaining 2 bytes can be used for a delay in milliseconds before the first frame is displayed, since the first frame's "Transition Time" can only be used as a "fade-in" effect from black, and it cannot be used for a "pure delay" unlike the "Frame Duration" value.