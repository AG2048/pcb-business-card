// AG Business Card 2025 V1 Processing Code
// This code reads the ST25DV64K NFC tag for display patterns, and displays them on the 6x12 RGB LED matrix. 

#include <Wire.h>
#include <tinyNeoPixel_Static.h>

// LED matrix configuration
#define LED_PIN 10
#define LED_ROWS 6
#define LED_COLS 12
#define LED_COUNT (LED_ROWS * LED_COLS)
#define MAX_BRIGHTNESS 63 // 255 is too bright, and power cannot support 72 LEDs at full brightness in white
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

void setup() {
  // Configure LED pin to output, needs to be done manually for tinyNeoPixel_Static
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Display the pattern on the LED matrix
  // TODO: This is currently a placeholder pattern generation. This works like a "progress bar"
  for (int i = 0; i < LED_COUNT * 3; i += 3) {
    pixel_data[i] = MAX_BRIGHTNESS;     // Red
    pixel_data[i + 1] = 0;   // Green
    pixel_data[i + 2] = 0;   // Blue
    leds.show(); // Update the LED matrix
    delay(100); // Delay for visibility
  }
  delay(1000); // Wait before repeating the pattern
  // All off
  for (int i = 0; i < LED_COUNT * 3; i++)
    pixel_data[i] = 0; // Turn off all LEDs
  leds.show(); // Update the LED matrix to turn off all LEDs
  delay(1000); // Wait before starting again
  // Cycle every LED from GRB
  for (int i = 0; i < LED_COUNT * 3; i++) {
    pixel_data[i] = MAX_BRIGHTNESS;
    leds.show(); // Update the LED matrix
    delay(100); // Delay for visibility
    pixel_data[i] = 0; // Turn off the LED after showing it
  }
  delay(1000); // Wait before repeating the cycle
  // All on, to test full brightness power
  for (int i = 0; i < LED_COUNT * 3; i++) {
    pixel_data[i] = MAX_BRIGHTNESS; // Set all LEDs to full brightness
  }
  leds.show(); // Update the LED matrix
  delay(1000); // Keep all LEDs on for visibility
  // All on but colours cycling
  for (int i = 0; i < LED_COUNT; i++) {
    pixel_data[i*3] = (i * (MAX_BRIGHTNESS+1) * (MAX_BRIGHTNESS+1) * (MAX_BRIGHTNESS+1) / LED_COUNT) & 63; // Red
    pixel_data[i*3 + 1] = ((i * (MAX_BRIGHTNESS+1) * (MAX_BRIGHTNESS+1) * (MAX_BRIGHTNESS+1) / LED_COUNT) >> 6) & 63; // Green
    pixel_data[i*3 + 2] = ((i * (MAX_BRIGHTNESS+1) * (MAX_BRIGHTNESS+1) * (MAX_BRIGHTNESS+1) / LED_COUNT) >> 12) & 63; // Blue
  }
  leds.show(); // Update the LED matrix
  delay(10000); // Keep the cycling colours for visibility
  // Make "AG" pattern
  leds_clear_matrix();
  // Set "A"
  for (int i = 0; i < 5; i++) {
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
  for (int i = 1; i < 5; i++) {
    leds_set_pixel_color(6, i, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  }
  for (int i = 0; i < 6; i++) {
    leds_set_pixel_color(7, i, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  }
  for (int i = 2; i < 5; i++) {
    leds_set_pixel_color(i+6, 5, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
  }
  for (int i = 2; i < 6; i++) {
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
}