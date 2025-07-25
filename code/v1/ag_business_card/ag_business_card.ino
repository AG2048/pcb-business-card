// AG Business Card 2025 V1 Processing Code
// This code reads the ST25DV64K NFC tag for display patterns, and displays them on the 6x12 RGB LED matrix. 

#include <Wire.h>
#include <tinyNeoPixel_Static.h>

// #################################################################################################################
// #                                      GLOBAL VARIABLES                                                         #
// #################################################################################################################

// LED matrix configuration
#define LED_PIN 10
#define LED_ROWS 6
#define LED_COLS 12

// NFC pin configuration
#define I2C_SDA 6
#define I2C_SCL 4
#define NFC_GPO 0

// NFC memory configuration
#define NFC_FIRST_ADDRESS 32 // The first address to read from the NFC tag, previous addresses are reserved for enabling url tap
#define NFC_MAX_POSSIBLE_FRAMES 627 // Max number of frames that can be stored on NFC tag

// #################################################################################################################
// #                                      LED MATRIX FUNCTIONS                                                     #
// #################################################################################################################

// LED matrix defines
#define LED_COUNT (LED_ROWS * LED_COLS)
#define MAX_BRIGHTNESS 15 // Using 4-bit color depth (0-15) for each RGB channel, as power cannot supply 255,255,255 brightness for all LEDs
#define PIXEL_ARRAY_SIZE (LED_COUNT * 3 / 2) // 1.5 bytes per pixel (4 bits per channel, 3 channels per pixel, 12 bits per pixel)
#define NUM_PIXELS_IN_BYTES (LED_COUNT / 8) // Number of pixels, represented in bytes. For single_colour display mode
byte pixel_data[PIXEL_ARRAY_SIZE]; // GRB444 data format, 1.5 bytes per pixel (4 bits per channel, 3 channels per pixel, 12 bits per pixel)

void leds_show() {
  /*
  This function sends the pixel_data stored into the RGB LED matrix.
  This function is similar to the `show()` function in the tinyNeoPixel library, but uses the static pixel_data array that only stores the lower 4 bits of each channel (upper bits are always 0).
  */
  //TODO
}

// LED Utility Functions
uint8_t pixel_index;
void leds_clear_matrix() {
  // Clear the LED matrix by setting all pixel data to zero, and show the update
  // This turns off all LEDs
  for (pixel_index = 0; pixel_index < PIXEL_ARRAY_SIZE; pixel_index++) {
    pixel_data[pixel_index] = 0; // Set all RGB values to 0
  }
  leds_show(); // Update the LED matrix to reflect the changes
}

void leds_set_pixel_color_by_coordinate(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b, byte *pixel_data) {
  // Set the color of a pixel at (x, y) position
  // Bottom-Left is (0, 0), top-right is (11, 5)
  // The LED matrix is ordered in a zigzag pattern: left-to-right on even rows, right-to-left on odd rows
  // The destination array can be pixel_data or any other byte array of size > PIXEL_ARRAY_SIZE
  if (x < LED_COLS && y < LED_ROWS) {
    pixel_index = (y * LED_COLS + (y % 2 == 0 ? x : (LED_COLS - 1 - x))); // Calculate the index of the pixel's location
    if (pixel_index % 2 != 0) {
      // If the pixel index is odd, then its first "G" channel is 2nd half of the first byte, and R/B channels are in the second byte
      pixel_index = pixel_index * 3 / 2;
      pixel_data[pixel_index] &= 0xF0; // Clear the lower 4 bits of the first byte (G channel)
      pixel_data[pixel_index] |= (g & 0x0F); // Set the lower 4 bits to G channel value
      pixel_data[pixel_index + 1] = (r & 0x0F) << 4; // Set the upper 4 bits of the second byte to R channel value
      pixel_data[pixel_index + 1] |= (b & 0x0F); // Set the lower 4 bits of the second byte to B channel value
    } else {
      // If the pixel index is even, then G/R channels are in the first byte, and B channel is in the second byte
      pixel_index = pixel_index * 3 / 2;
      pixel_data[pixel_index] = (g & 0x0F) << 4; // Set the upper 4 bits of the first byte to G channel value
      pixel_data[pixel_index] |= (r & 0x0F); // Set the lower 4 bits of the first byte to R channel value
      pixel_data[pixel_index + 1] &= 0x0F; // Clear the upper 4 bits of the second byte (B channel)
      pixel_data[pixel_index + 1] |= (b & 0x0F) << 4; // Set the upper 4 bits of the second byte to B channel value
    }
  }
}

void leds_set_pixel_color_by_index(uint8_t index, uint8_t r, uint8_t g, uint8_t b, byte *pixel_data) {
  // Set the color of a pixel at the specified index
  // Index is from 0 to LED_COUNT - 1
  // The destination array can be pixel_data or any other byte array of size > PIXEL_ARRAY_SIZE
  if (index < LED_COUNT) {
    pixel_index = index * 3 / 2; // Calculate the index in the pixel_data array
    if (index % 2 != 0) {
      // If the index is odd, then its first "G" channel is 2nd half of the first byte, and R/B channels are in the second byte
      pixel_data[pixel_index] &= 0xF0; // Clear the lower 4 bits of the first byte (G channel)
      pixel_data[pixel_index] |= (g & 0x0F); // Set the lower 4 bits to G channel value
      pixel_data[pixel_index + 1] = (r & 0x0F) << 4; // Set the upper 4 bits of the second byte to R channel value
      pixel_data[pixel_index + 1] |= (b & 0x0F); // Set the lower 4 bits of the second byte to B channel value
    } else {
      // If the index is even, then G/R channels are in the first byte, and B channel is in the second byte
      pixel_data[pixel_index] = (g & 0x0F) << 4; // Set the upper 4 bits of the first byte to G channel value
      pixel_data[pixel_index] |= (r & 0x0F); // Set the lower 4 bits of the first byte to R channel value
      pixel_data[pixel_index + 1] &= 0x0F; // Clear the upper 4 bits of the second byte (B channel)
      pixel_data[pixel_index + 1] |= (b & 0x0F) << 4; // Set the upper 4 bits of the second byte to B channel value
    }
  }
}

void leds_fill_solid_red() {
  // Fill the entire LED matrix with solid red color. Always to the pixel_data array.
  for (pixel_index = 0; pixel_index < PIXEL_ARRAY_SIZE; pixel_index += 3) {
    pixel_data[pixel_index] = 0x0F; // Set G channel to 0, R channel to 15
    pixel_data[pixel_index + 1] = 0x00; // Set B channel to 0
    pixel_data[pixel_index + 2] = 0xF0; // Set R channel to 15, B channel to 0

  }
  leds_show(); // Update the LED matrix to reflect the changes
}

void leds_copy_from_buffer(byte *buffer) {
  /*
  This function copies the pixel data from the provided buffer to the pixel_data array.
  The buffer should be in the same format as pixel_data (GRB444).
  Always copy to pixel_data array.
  */
  for (pixel_index = 0; pixel_index < PIXEL_ARRAY_SIZE; pixel_index++) {
    pixel_data[pixel_index] = buffer[pixel_index]; // Copy each byte from the buffer to pixel_data
  }
}

// #################################################################################################################
// #                                             NFC FUNCTIONS                                                     #
// #################################################################################################################

int NFC_I2C_ADDRESS = 0b1010011; // ST25DV64K NFC tag I2C address

byte nfc_data[128]; // Buffer for NFC data
byte nfc_address_upper_byte; // Upper byte of the NFC memory address
byte nfc_address_lower_byte; // Lower byte of the NFC memory address
int nfc_index; // Iterator for NFC data

bool nfc_read_data(int address, int length) {
  /*
  This function reads data from the NFC tag at the specified memory address, and stores it in the nfc_data buffer.
  Parameters:
    - address: The memory address to read from (0-8191) for ST25DV64K with 8kB of memory.
    - length: The number of bytes to read (1-128), length of 0 will return false.
  Returns:
    - true if data was read successfully, false otherwise.
  */

  Wire.beginTransmission(NFC_I2C_ADDRESS); // Open I2C session with a write operation
  nfc_address_upper_byte = (address >> 8) & 0xFF; // Get upper byte of address
  nfc_address_lower_byte = address & 0xFF; // Get lower byte of address
  Wire.write(nfc_address_upper_byte); // Write upper address byte.
  Wire.write(nfc_address_lower_byte); // Write lower address byte
  if (Wire.endTransmission(false)) return false; // Do not release the bus, send RESTART condition. Return false if transmission failed (return value != 0)

  Wire.requestFrom(NFC_I2C_ADDRESS, length); // Request data from NFC tag in read mode
  if (Wire.available() == length) {
    for (nfc_index = 0; nfc_index < length; nfc_index++) {
      nfc_data[nfc_index] = Wire.read(); // Read data into buffer
    }
    return true; // Data read successfully
  }
  return false; // Failed to read data
}

// #################################################################################################################
// #                                             UTIL FUNCTIONS                                                    #
// #################################################################################################################

// Global variables for storing the current state of the NFC tag and LED matrix
uint16_t num_frames = 0; // Variable to remember number of frames are stored in the NFC tag
byte color_mode = 0; // Only lowest 3 bits used for color mode. 0 = GRB444, rest from 001 to 111 are corresponding to GRB channel max brightness (e.g. 001 = B, 011 = R+B, 111 = R+G+B)
uint16_t transition_time = 0; // Transition time in milliseconds for the LED matrix pattern from the previous frame after it has finished displaying
uint16_t frame_duration = 0; // Duration in milliseconds for the LED matrix pattern to be displayed before transitioning to the next frame
uint16_t initial_delay = 0; // Initial delay in milliseconds before the first frame begins transitioning
int current_nfc_address = NFC_FIRST_ADDRESS; // Current NFC address to read from, starts at NFC_FIRST_ADDRESS

unsigned long start_time = 0; // Start time of the timer
unsigned long current_time = 0; // Current time in milliseconds

bool initialize() {
  /*
  This function runs at beginning of loop() to read the basic parameters from the NFC tag. 
  Reads the number of frames (stored in first 2 bytes)
  Reads the initial delay (stored in next 2 bytes)
  Clears the pixel data buffer
  Returns true if initialization was successful, false otherwise.
  */

  leds_clear_matrix(); // Clear the LED matrix before starting

  delay(500); // Wait for NFC tag to power up

  if (!nfc_read_data(current_nfc_address, 4)) {
    leds_fill_solid_red(); // Fill the LED matrix with solid red color to indicate error
    return false; // Failed to read data from NFC tag
  }

  num_frames = (nfc_data[0] << 8) | nfc_data[1]; // Read number of frames from the first 2 bytes
  initial_delay = (nfc_data[2] << 8) | nfc_data[3]; // Read initial delay from the next 2 bytes

  current_nfc_address += 4; // Move to the next address for reading frame data

  delay(initial_delay); // Wait for the initial delay before starting to display frames
}

bool load_and_display_frame() {
  /*
  This function reads the frame configuration at current_nfc_address.
  It reads the color mode, transition time, and frame duration from the NFC tag.
  The function then loads the frame pixels into the nfc_data buffer.
  This function calls the appropriate display_frame_*() function based on the color mode.
  Returns true if the frame was loaded successfully, false otherwise.
  */

  // Load the 4-byte frame configuration from the NFC tag
  if (!nfc_read_data(current_nfc_address, 4)) {
    leds_fill_solid_red(); // Fill the LED matrix with solid red color to indicate error
    return false; // Failed to read frame data from NFC tag
  }

  color_mode = (nfc_data[0] >> 5) & 0x07; // Colour mode is the highest 3 bits of the first byte
  transition_time = ((nfc_data[0] & 0x1F) << 8) | nfc_data[1]; // Transition time is the lower 5 bits of the first byte and the second byte
  frame_duration = (nfc_data[2] << 8) | nfc_data[3]; // Frame duration is the third and fourth bytes

  // Everything successful, update address for the actual frame data
  current_nfc_address += 4;

  // display the frame based on the color mode
  if (color_mode == 0) {
    // GRB444 color mode
    if (!display_frame_grb444()) {
      current_nfc_address -= 4; // Rollback the address to the current frame configuration
      leds_fill_solid_red(); // Fill the LED matrix with solid red color to indicate error
      return false; // Failed to display frame
    }
  } else {
    // Solid color mode
    if (!display_frame_solid_color()) {
      current_nfc_address -= 4; // Rollback the address to the current frame configuration
      leds_fill_solid_red(); // Fill the LED matrix with solid red color to indicate error
      return false; // Failed to display frame
    }
  }

  num_frames--; // Decrease the number of frames left to display

  return true; // Frame loaded successfully
}

bool display_frame_grb444() {
  /*
  This function displays the frame stored in nfc_data using the GRB444 color mode, and the transition_time and frame_duration parameters.
  This colour mode involves directly copying the pixel data from nfc_data to pixel_data.
  Before calling handle_transition(), it stores the next frame's pixel data in the nfc_data buffer.
  Returns true if the frame was displayed successfully, false otherwise.
  */
  if (!nfc_read_data(current_nfc_address, PIXEL_ARRAY_SIZE)) {
    return false; // Failed to read pixel data from NFC tag
  }
  current_nfc_address += PIXEL_ARRAY_SIZE; // Move to the next address for the next frame configuration
  if (handle_transition()) {
    // Transition is handled correctly. Just wait for the frame duration
    delay(frame_duration); // Wait for the frame duration before transitioning to the next frame
    return true; // Frame displayed successfully
  } else {
    // Transition failed, rollback the address to the current frame configuration
    current_nfc_address -= PIXEL_ARRAY_SIZE; // Rollback to the current frame configuration
    return false; // Failed to handle transition
  }
}

byte solid_color_r = 0; // Red channel value for solid color frame
byte solid_color_g = 0; // Green channel value for solid color frame
byte solid_color_b = 0; // Blue channel value for solid color frame
byte temp_pixel_data[NUM_PIXELS_IN_BYTES]; // Temporary pixel data buffer for solid color frame
bool display_frame_solid_color() {
  /*
  This function displays a solid color frame based on the color mode and the transition_time and frame_duration parameters.
  The color mode is determined by the lowest 3 bits of the color_mode variable.
  Before calling handle_transition(), it stores the next frame's pixel data in the nfc_data buffer.
  Returns true if the frame was displayed successfully, false otherwise.
  */
  if (!nfc_read_data(current_nfc_address, NUM_PIXELS_IN_BYTES)) {
    return false; // Failed to read solid color data from NFC tag
  }
  current_nfc_address += NUM_PIXELS_IN_BYTES; // Move to the next address for the next frame configuration
  // Extract the solid color values from the nfc_data buffer based on the color mode
  solid_color_r = 0;
  solid_color_g = 0;
  solid_color_b = 0;
  if (color_mode & 0b001) {
    solid_color_r = MAX_BRIGHTNESS;
  }
  if (color_mode & 0b010) {
    solid_color_g = MAX_BRIGHTNESS;
  }
  if (color_mode & 0b100) {
    solid_color_b = MAX_BRIGHTNESS;
  }
  // Move data to temp_pixel_data buffer
  for (pixel_index = 0; pixel_index < NUM_PIXELS_IN_BYTES; pixel_index++) {
    temp_pixel_data[pixel_index] = nfc_data[pixel_index]; // Copy each byte from the nfc_data to temp_pixel_data
  }

  // Fill nfc_data with the solid color values based on the color mode
  for (pixel_index = 0; pixel_index < LED_COUNT; pixel_index++) {
    if (temp_pixel_data[pixel_index / 8] & (1 << (7-(pixel_index % 8)))) { // MSB is first pixel
      // If the pixel is set, set the color to solid color
      leds_set_pixel_color_by_index(pixel_index, solid_color_r, solid_color_g, solid_color_b, nfc_data);
    } else {
      // If the pixel is not set, turn it off
      leds_set_pixel_color_by_index(pixel_index, 0, 0, 0, nfc_data);
    }
  }

  if (handle_transition()) {
    // Transition is handled correctly. Just wait for the frame duration
    delay(frame_duration); // Wait for the frame duration before transitioning to the next frame
    return true; // Frame displayed successfully
  } else {
    // Transition failed, rollback the address to the current frame configuration
    current_nfc_address -= NUM_PIXELS_IN_BYTES; // Rollback to the current frame configuration
    return false; // Failed to handle transition
  }
}

byte original_pixel_data[PIXEL_ARRAY_SIZE]; // Buffer to store the original pixel data for transition handling
bool handle_transition() {
  /*
  This function handles the transition between frames.
  It updates the pixel data based on the transition_time and the difference between the current frame in pixel_data and the next frame in nfc_data.
  Returns true if the transition was handled successfully, false otherwise.
  */
  // We can make a 3rd buffer array to store the original frame's data. Then every 16 ms, we calculate what the next pixel data should be. In the end, we copy the pixel_data to the nfc_data buffer.
  for (pixel_index = 0; pixel_index < PIXEL_ARRAY_SIZE; pixel_index++) {
    original_pixel_data[pixel_index] = pixel_data[pixel_index]; // Copy the current pixel data to the original_pixel_data buffer
  }

  start_time = millis(); // Reset the frame start time

  // TODO: Implement the transition logic

  // In the end, always copy from nfc_data to pixel_data to ensure the frame is correctly displayed
  leds_copy_from_buffer(nfc_data); // Copy the pixel data from nfc_data to pixel_data
  leds_show(); // Show the initial frame
  return true;
}

// #################################################################################################################
// #                                             MAIN FUNCTIONS                                                    #
// #################################################################################################################

void setup() {
  // Configure LED pin to output, needs to be done manually for tinyNeoPixel_Static
  pinMode(LED_PIN, OUTPUT);

  // Configure I2C for NFC
  Wire.begin();
}

void loop() {
  if (!initialize()) {
    // If initialization failed, repeat loop(). initialize() have already filled the LED matrix with solid red color to indicate error
    return;
  }

  // If there are frames to display, start the display loop
  while (num_frames) {
    if (!load_and_display_frame()) {
      // If loading the frame failed, repeat this until it either succeeds or user debugs the issue.
      continue;
    }
  }
}