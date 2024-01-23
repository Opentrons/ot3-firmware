#pragma once

#include <stdint.h>

#include "hepa-uv/core/constants.h"

// The frequency of one full PWM cycle
#define LED_PWM_FREQ_HZ (2000UL)
// the frequency at which the timer should count so that it
// does a full PWM cycle in the time specified by LED_PWM_FREQ_HZ
#define LED_TIMER_FREQ (LED_PWM_FREQ_HZ * LED_PWM_WIDTH)


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void set_button_led_pwm(PUSH_BUTTON_TYPE button, uint32_t red, uint32_t green, uint32_t blue, uint32_t white);
void set_hepa_fan_pwm(uint32_t duty_cycle);
void initialize_hepauv_hardware();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
