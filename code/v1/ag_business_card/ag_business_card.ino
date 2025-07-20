// AG Business Card 2025 V1 Processing Code
// This code reads the ST25DV64K NFC tag for display patterns, and displays them on the 6x12 RGB LED matrix. 

#include <Wire.h>
#include <tinyNeoPixel_Static.h>

// LED matrix configuration
#define LED_PIN 10
#define LED_COUNT 72
byte pixel_data[LED_COUNT * 3];
tinyNeoPixel leds = tinyNeoPixel(LED_COUNT, LED_PIN, NEO_GRB, pixel_data);

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
    pixel_data[i] = 255;     // Red
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
    pixel_data[i] = 255;
    leds.show(); // Update the LED matrix
    delay(100); // Delay for visibility
    pixel_data[i] = 0; // Turn off the LED after showing it
  }
  delay(1000); // Wait before repeating the cycle
  // All on, to test full brightness power
  for (int i = 0; i < LED_COUNT * 3; i++) {
    pixel_data[i] = 255; // Set all LEDs to full brightness
  }
  leds.show(); // Update the LED matrix
  delay(1000); // Keep all LEDs on for visibility
}