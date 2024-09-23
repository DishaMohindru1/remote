/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "pico/stdlib.h"  // Standard I/O for Pico SDK (to control GPIOs, etc.)
#include "tusb.h"         // TinyUSB library for USB communication
#include "pico_hid.h"     // Custom header for gamepad HID reports
#include "hardware/adc.h" // Library to interact with the Analog-to-Digital Converter (ADC)

// Button configuration for the gamepad
typedef struct {
  uint32_t action;    // Button action (e.g., GAMEPAD_BUTTON_SOUTH)
  uint8_t gpio_pin;   // GPIO pin associated with the button
} button_config_t;

// Joystick map to store the ADC values for X and Y axes
typedef struct {
  uint16_t x;         // X-axis joystick value
  uint16_t y;         // Y-axis joystick value
} joystick_t;

joystick_t joystick;   // Declare joystick for X and Y axes

// Button configuration with GPIO pin mapping
const button_config_t button_config[] = {
  {GAMEPAD_BUTTON_SOUTH, 7},   // South button on GPIO 7
  {GAMEPAD_BUTTON_EAST, 8},    // East button on GPIO 8
  {GAMEPAD_BUTTON_NORTH, 5},   // North button on GPIO 5
  {GAMEPAD_BUTTON_WEST, 6},    // West button on GPIO 6
  {GAMEPAD_BUTTON_MODE, 9},    // Mode button on GPIO 9
  {GAMEPAD_BUTTON_SELECT, 20}, // Select button on GPIO 20
  {GAMEPAD_BUTTON_START, 21}   // Start button on GPIO 21
};

const int button_count = sizeof(button_config) / sizeof(button_config[0]);  // Total number of buttons

// Setup GPIO for buttons and initialize ADC for joystick
void setup_controller_buttons(void) {
  for (int i = 0; i < button_count; i++) {
    gpio_init(button_config[i].gpio_pin);        // Initialize GPIO pin
    gpio_set_dir(button_config[i].gpio_pin, GPIO_IN);  // Set pin as input
    gpio_pull_up(button_config[i].gpio_pin);     // Enable pull-up resistor
  }

  adc_init();             // Initialize ADC hardware for joystick
  adc_gpio_init(26);      // GPIO 26 connected to joystick X-axis (ADC input)
  adc_gpio_init(27);      // GPIO 27 connected to joystick Y-axis (ADC input)
}

// Check if the HID report is empty (no input detected)
bool is_empty(const hid_gamepad_report_t *report) {
  return report->buttons == 0 && report->x == 0 && report->y == 0;
}

// Update HID report with button states and joystick positions
void update_hid_report_controller(hid_gamepad_report_t *report) {
  // Check button states and update the report
  for (int i = 0; i < button_count; i++) {
    if (!gpio_get(button_config[i].gpio_pin)) {
      report->buttons |= button_config[i].action;  // Set the button if pressed
    }
  }

  // Read joystick X and Y values from ADC
  adc_select_input(0);                 // Select ADC input 0 (X-axis)
  joystick.x = adc_read();             // Read X-axis value (12-bit)

  adc_select_input(1);                 // Select ADC input 1 (Y-axis)
  joystick.y = adc_read();             // Read Y-axis value (12-bit)

  // Scale 12-bit ADC values to 8-bit range for HID report
  report->x = joystick.x / 16;  // Scale X-axis to 8-bit
  report->y = joystick.y / 16;  // Scale Y-axis to 8-bit
}
