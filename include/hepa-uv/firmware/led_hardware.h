#pragma once

#include <stdint.h>

#include "hepa-uv/core/constants.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void button_led_hw_update_pwm(uint32_t duty_cycle, LED_TYPE led, PUSH_BUTTON_TYPE button);
void set_button_led_pwm(PUSH_BUTTON_TYPE button, uint32_t red, uint32_t green, uint32_t blue, uint32_t white);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
