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
   - Adding 4 extra 0 writes in front of bit 7 and bit 3 makes the assembly code too long that the relative jump at the end of each byte write cannot reach the beginning of the program. (64 instructions limit per jump)
   - Broken the code into a write upper bits, write lower bits, and two separate write-zero functions. They are arranged in: WRITE_UPPER -> WRITE_ZERO_TO_LOWER -> WRITE_ZERO_TO_UPPER -> WRITE_LOWER.
   - The program starts the write-zero that jumps to write upper bits. Then it proceeds to the write-zero that jumps to write lower bits. Finally, it jumps back to the write zero that jumps to write upper bits again. The program loops in a sort of "zig-zag" pattern.
   - This breaks the jumps into smaller 50-ish instructions jumps, allowing the assembly code to function within the timing constraints of the LEDs, since there is not a lot of free instructions left to fit every bit write within 10 cycles. 

A custom leds_set_pixel_color function is used to set the pixel color in the `pixel_data` array. This function takes the x and y coordinates of the pixel, and the red, green, and blue values to set the pixel color. This is used because the LEDs are arranged in a zig-zag pattern, so the pixel data needs to be set in a specific order.

## NFC I2C
The ST25DV64K NFC tag is used to store the display patterns for the RGB LED matrix. The NFC tag is accessed using the I2C protocol, and the `Wire` library is used to communicate with the NFC tag.

Currently it is assumed that the NFC tag does not have password protection enabled for wired I2C access.

The ATTiny84 will wait for 500 ms at the start of the program to allow the NFC tag to power up and be ready for communication. This is done using `delay(500)`.

To read the NFC tag, the `nfc_read_data` function is used. This function reads a specified number of bytes from the NFC tag starting at a specified address. The data is stored in the `nfc_data` buffer array.

Inside `nfc_read_data`, we first use `Wire.beginTransmission(NFC_I2C_ADDRESS)` to begin a `WRITE` operation to the NFC tag, where we write the 2-byte address we wish to read from (upper byte first). According to the ST25DV64K datasheet, we need to send a `RESTART` condition and switch to a `READ` operation to read the data from memory. This was done using `Wire.endTransmission(false)` to keep the I2C bus open, and then using `Wire.requestFrom(NFC_I2C_ADDRESS, length)` to read the data from the NFC tag.

`NFC_I2C_ADDRESS` is defined as `0b1010011`, which is the NFC tag's address that accesses the user memory. 

## Pattern Encoding
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
- `T_TTTT_TTTT_TTTT` is the transition time (13 bits)
- `DDDD_DDDD_DDDD_DDDD` is the frame duration (16 bits)

This aligns with the 32-bit word size of the NFC tag.

With the metadata, each frame will have a minimum of `4+9=13` bytes for single pixel mode, or `4+108=112` bytes for `GRB444` mode. So we can fit a minimum of 72 frames to a maximum of 627 frames. 

The first few bytes should store some data about the overall storage. Such as:
- The number of images/frames stored in the NFC tag -- so that the code knows when to stop reading. 
  - This uses 2 bytes as maximum is 627 > 255 for 1 byte storage. 
- The remaining 2 bytes can be used for a delay in milliseconds before the first frame is displayed, since the first frame's "Transition Time" can only be used as a "fade-in" effect from black, and it cannot be used for a "pure delay" unlike the "Frame Duration" value.

Overall, the first 4 bytes of the NFC tag should be:
`NNNN_NNNN_FFFF_FFFF`
Where:
- `NNNN_NNNN` is the number of frames stored (8 bits)
- `FFFF_FFFF` is the delay before the first frame (32 bits)

## Code Flow
The code proceeds as follows:
1. Initialize
  - Set memory address to `NFC_FIRST_ADDRESS` (32, block 8)
  - Read `NNNN_NNNN` and `FFFF_FFFF` from the NFC tag to get the number of frames and the initial delay.
  - Wait for the initial delay before displaying the first frame.
2. Load frame data
  - Read `CCC`, `T_TTTT_TTTT_TTTT`, and `DDDD_DDDD_DDDD_DDDD` from the NFC tag to get the colour mode, transition time, and frame duration.
  - If colour mode is `GRB444` (`CCC == 000`) display in `GRB444` mode.
  - If colour mode is single pixel mode, display in single pixel mode.
3. Display frame
  - `GRB444` mode:
    - Read `PIXEL_ARRAY_SIZE` bytes from the NFC tag, starting from the current address.
    - Use `nfc_data` as a buffer for the next frame.
    - Handle transition and display. 
    - Wait for frame duration.
  - Single pixel mode:
    - Read `NUM_PIXELS_IN_BYTES` bytes from the NFC tag, starting from the current address.
    - Depending on which bit in `CCC` is set, set that channel's colour to `MAX_BRIGHTNESS`. Else set to `0`. 
    - Based on each bit is `1` or `0`, fill each pixel by 0 or the colours. 
    - Use `nfc_data` as a buffer for the next frame.
    - Handle transition and display.
    - Wait for frame duration.
4. Handle transition
  - If the transition time is `0`, display the next frame immediately (prevent divide by 0 errors).
  - If the transition time is greater than `0`, fade from the current frame to the next frame over the specified transition time.
  - Store original frame in `original_pixel_data` array. 
  - Every 16 ms, calculate the "gap" between pixel values, and interpolate pixel values based on elapsed time and transition time.
  - Update pixel data in `pixel_data` array and display the frame.
  - At the end, copy the entire `nfc_data` to `pixel_data` to complete the transition.
5. Repeat
  - Repeat 2-4 until all frames from `NNNN_NNNN` have been displayed.
  - Repeat step 1. 

During this time, if any error occurs (most likely NFC I2C read error), the code will flash a full-red screen with no delay. The program restarts from initialization. This is done to prevent address overflowing / wrapping on an uninitialized NFC memory. 

## Function / Variable / Define Documentation
Note: all pins are using the default clockwise pin numbering from `ATTinyCore`. 
### Hardware Configuration
#### `#define LED_PIN`
The pin number for the RGB LED matrix. This is connected to pin 10 of the ATTiny84.
#### `#define LED_ROWS`
The number of rows in the RGB LED matrix. This is set to 6 for the 6x12 matrix.
#### `#define LED_COLS`
The number of columns in the RGB LED matrix. This is set to 12 for the 6x12 matrix.
#### `#define I2C_SDA`
The I2C SDA pin number. This is connected to pin 6 of the ATTiny84.
#### `#define I2C_SCL`
The I2C SCL pin number. This is connected to pin 4 of the ATTiny84.
#### `#define NFC_GPO`
The NFC GPO pin number. This is connected to pin 0 of the ATTiny84.
#### `#define NFC_FIRST_ADDRESS`
The first address in the NFC tag to read from. This is set to 32, which is block 8 in the ST25DV64K NFC tag memory. This is where the pixel data starts.
#### `#define NFC_MAX_POSSIBLE_FRAMES`
The maximum number of frames that can be stored in the NFC tag. This is set to 627, which is the maximum number of frames that can be stored in the NFC tag with the current metadata format. This is currently not used. 
### LED Matrix
#### `#define LED_COUNT`
The number of LEDs in the RGB LED matrix. This is set to 72 (`LED_ROWS * LED_COLS`).
#### `#define MAX_BRIGHTNESS`
The maximum brightness value for each colour channel in the RGB LED matrix. This is set to 15 (0xF) to prevent excess power draw and brightness.
#### `#define PIXEL_ARRAY_SIZE`
The size of the pixel array in bytes for `GRB444` mode. This is set to `LED_COUNT * 1.5`, which is 108 bytes for 72 pixels, since each pixel takes up 12 bits in `GRB444` format.
#### `#define NUM_PIXELS_IN_BYTES`
The number of pixels divided by 8. This is used to calculate the size of a buffer required to store bytes that represent the on/off state of each pixel for each bit of data.
#### `byte pixel_data[PIXEL_ARRAY_SIZE]`
The pixel data array that stores the `GRB444` pixel data for the RGB LED matrix. This is a static array of bytes that is used to store the pixel data for the current frame. The `leds_show()` function uses this static array to write data to the RGB LED matrix.
#### `uint32_t endTime`
The end time when the previous frame was finished displaying. This is used to determine if sufficient time has passed since the last frame's data was sent, enough for the reset time to register on the LEDs. The value is set to `micros()` at the end of each `leds_show()` call, and the `leds_show()` function will wait until `micros() - endTime` is greater than or equal to `50L` before proceeding to the next frame.
#### `uint8_t p = LED_PIN`
The pin number for the LEDs. This is used by `digitalPinToPort()` and `digitalPinToBitMask()` to get the `port` and `pinMask` for the LED pin. 
#### `volatile uint8_t *port`
Pointer to the port output register for the LED pin. This is used in assembly code to directly write to the output port for faster performance.
#### `uint8_t pinMask`
The bit mask for the LED output pin. This is used to set `hi` and `lo` values to set the output pin high or low in the assembly code.

WIP, TODO. 

## Generate NFC Binary
The `generate_nfc_binary.py` script is used to generate the binary file that can be written to the NFC tag to be displayed.
It takes a list of images or GIFs, and associated colour and timing data, and generates a hex binary file for the NFC tag.