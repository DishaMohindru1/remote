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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tusb.h"
#include <pico/stdlib.h>
#include "pico_hid.h"

#ifndef JUST_STDIO
#include "bsp/board.h"
#include "usb_descriptors.h"
#endif

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

void hid_task(void);  // Task to handle USB HID (Human Interface Device)

/*------------- MAIN -------------*/
int main(void)
{
  #ifndef JUST_STDIO
  board_init();   
  tusb_init();    
  #else
  stdio_init_all();  
  #endif

  setup_controller_buttons();  // Configure the buttons and joystick as input devices

  // Infinite loop to continuously process USB and gamepad tasks
  while (1)
  {
    #ifndef JUST_STDIO
    tud_task(); // TinyUSB device task (handles USB requests from the host)
    hid_task();  // Task to handle Human Interface Device (HID) report generation and transmission
    #else
    hid_gamepad_report_t report =
    {
        .x   = 0, 
        .y   = 0, 
        .buttons = 0 
    };
    
    update_hid_report_controller(&report);
    printf("hat: %d buttons: %d\n", report.hat, report.buttons);  
    #endif
  }

  return 0;  
}

//--------------------------------------------------------------------+
// USB HID (Human Interface Device) - USB Communication
//--------------------------------------------------------------------+

#ifndef JUST_STDIO

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  if ( !tud_hid_ready() ) return;  

  switch(report_id)
  {
    case REPORT_ID_GAMEPAD:  
    {
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, 
        .y   = 0, 
        .buttons = 0 
      };

      update_hid_report_controller(&report);

      if ( !is_empty(&report) )
      {
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));  
        has_gamepad_key = true;  
      }
      else if (has_gamepad_key)
      {
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;  
      }
    }
    break;

    default: break;  
  }
}

void hid_task(void)
{
  const uint32_t interval_ms = 10;  
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms ) return;  
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();  

  send_hid_report(REPORT_ID_GAMEPAD, btn);  // Send the gamepad report to the host
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
}

#endif




