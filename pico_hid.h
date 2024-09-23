// #define JUST_STDIO

#include "tusb.h"

void setup_controller_buttons(void);
bool is_empty(const hid_gamepad_report_t *report);
void update_hid_report_controller(hid_gamepad_report_t *report);

