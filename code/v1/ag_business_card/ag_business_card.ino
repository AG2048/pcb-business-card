// AG Business Card 2025 V1 Processing Code
// This code reads the ST25DV64K NFC tag for display patterns, and displays them on the 6x12 RGB LED matrix. 

#include <Wire.h>
#include <tinyNeoPixel_Static.h>

int i; // iteration variable

// LED matrix configuration
#define LED_PIN 10
#define LED_ROWS 6
#define LED_COLS 12
#define LED_COUNT (LED_ROWS * LED_COLS)
#define MAX_BRIGHTNESS 15 // 255 is too bright, and power cannot support 72 LEDs at full brightness in white
byte pixel_data[LED_COUNT * 3];
tinyNeoPixel leds = tinyNeoPixel(LED_COUNT, LED_PIN, NEO_GRB, pixel_data);

// LED Utility Functions
int pixel_index;
void leds_clear_matrix() {
  // Clear the LED matrix by setting all pixel data to zero
  for (pixel_index = 0; pixel_index < LED_COUNT * 3; pixel_index++) {
    pixel_data[pixel_index] = 0; // Set all RGB values to 0
  }
  leds.show(); // Update the LED matrix to reflect the changes
}
void leds_set_pixel_color(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
  // Set the color of a pixel at (x, y) position
  // Bottom-Left is (0, 0), top-right is (11, 5)
  // The LED matrix is ordered in a zigzag pattern: left-to-right on even rows, right-to-left on odd rows
  if (x < LED_COLS && y < LED_ROWS) {
    pixel_index = (y * LED_COLS + (y % 2 == 0 ? x : (LED_COLS - 1 - x))) * 3; // Calculate pixel index
    pixel_data[pixel_index] = r;     // Red
    pixel_data[pixel_index + 1] = g; // Green
    pixel_data[pixel_index + 2] = b; // Blue
  }
}

// NFC configuration
#define I2C_SDA 6
#define I2C_SCL 4
#define NFC_GPO 0
uint8_t NFC_I2C_ADDRESS = 0b1010011;
byte nfc_data[128]; // Buffer for NFC data
byte nfc_address_upper_byte;
byte nfc_address_lower_byte;
bool nfc_read_data(int address, uint8_t length) {
  Wire.beginTransmission(NFC_I2C_ADDRESS); // Open I2C session with a write operation
  nfc_address_upper_byte = (address >> 8) & 0xFF; // Get upper byte of address
  nfc_address_lower_byte = address & 0xFF; // Get lower byte of address
  Wire.write(nfc_address_upper_byte); // Write upper address byte.
  Wire.write(nfc_address_lower_byte); // Write lower address byte
  if (Wire.endTransmission(false)) return false; // Do not release the bus, send RESTART condition. Return false if transmission failed (return value != 0)

  Wire.requestFrom(NFC_I2C_ADDRESS, length); // Request data from NFC tag in read mode
  if (Wire.available() == length) {
    for (i = 0; i < length; i++) {
      nfc_data[i] = Wire.read(); // Read data into buffer
    }
    return true; // Data read successfully
  }
  return false; // Failed to read data
}

void setup() {
  // Configure LED pin to output, needs to be done manually for tinyNeoPixel_Static
  pinMode(LED_PIN, OUTPUT);

  // Configure I2C for NFC
  Wire.begin();
}

void loop() {
  // Display the pattern on the LED matrix
  // TODO: This is currently a placeholder pattern generation. This works like a "progress bar"
  for (i = 0; i < LED_COUNT * 3; i += 3) {
    pixel_data[i] = MAX_BRIGHTNESS;     // Red
    pixel_data[i + 1] = 0;   // Green
    pixel_data[i + 2] = 0;   // Blue
    leds.show(); // Update the LED matrix
    delay(10); // Delay for visibility
  }
  delay(1000); // Wait before repeating the pattern
  // All off
  for (i = 0; i < LED_COUNT * 3; i++)
    pixel_data[i] = 0; // Turn off all LEDs
  leds.show(); // Update the LED matrix to turn off all LEDs
  delay(1000); // Wait before starting again
  // Cycle every LED from GRB
  for (i = 0; i < LED_COUNT * 3; i++) {
    pixel_data[i] = MAX_BRIGHTNESS;
    leds.show(); // Update the LED matrix
    delay(100); // Delay for visibility
    pixel_data[i] = 0; // Turn off the LED after showing it
  }
  delay(1000); // Wait before repeating the cycle
  // All on, to test full brightness power
  for (i = 0; i < LED_COUNT * 3; i++) {
    pixel_data[i] = MAX_BRIGHTNESS; // Set all LEDs to full brightness
  }
  leds.show(); // Update the LED matrix
  delay(1000); // Keep all LEDs on for visibility
  // All on but colours cycling
  for (i = 0; i < LED_COUNT; i++) {
    pixel_data[i*3] = (i * 46) & 15; // Red
    pixel_data[i*3 + 1] = ((i * 46) >> 4) & 15; // Green
    pixel_data[i*3 + 2] = ((i * 46) >> 8) & 15; // Blue
  }
  leds.show(); // Update the LED matrix
  delay(10000); // Keep the cycling colours for visibility
  // Make "AG" pattern
  leds_clear_matrix();
  // Set "A"
  for (i = 0; i < 5; i++) {
    leds_set_pixel_color(0, i, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
    leds_set_pixel_color(1, i, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
    leds_set_pixel_color(4, i, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
    leds_set_pixel_color(5, i, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  }
  leds_set_pixel_color(2, 2, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(3, 2, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(1, 5, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(2, 5, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(3, 5, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(4, 5, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  // Set "G"
  for (i = 1; i < 5; i++) {
    leds_set_pixel_color(6, i, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  }
  for (i = 0; i < 6; i++) {
    leds_set_pixel_color(7, i, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  }
  for (i = 2; i < 5; i++) {
    leds_set_pixel_color(i+6, 5, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  }
  for (i = 2; i < 6; i++) {
    leds_set_pixel_color(i+6, 0, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  }
  leds_set_pixel_color(4+6, 1, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(5+6, 1, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(4+6, 2, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(5+6, 2, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(3+6, 2, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(4+6, 4, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  leds_set_pixel_color(5+6, 4, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  // Show the "AG" pattern
  leds.show();
  delay(5000); // Keep the "AG" pattern for visibility
  leds_clear_matrix();

  // Read NFC data and display it (1 is light, 0 is dark)
  nfc_read_data(32, 9); // Read 9 bytes from NFC tag starting at address 32 (for 72 bits)
  for (i = 0; i < 72; i++) {
    if (nfc_data[i / 8] & (1 << (7 - (i % 8)))) { // Check if the bit is set (MSB of a byte first)
      leds_set_pixel_color(i % LED_COLS, i / LED_COLS, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS); // Set pixel to white
    } else {
      leds_set_pixel_color(i % LED_COLS, i / LED_COLS, 0, 0, 0); // Set pixel to off
    }
  }
  leds.show(); // Update the LED matrix to display the NFC data
  delay(10000); // Keep the NFC data display for visibility
  leds_clear_matrix(); // Clear the matrix after displaying NFC data
  delay(1000); // Wait before starting the loop again
}