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

void led_hw_update_pwm(uint32_t duty_cycle, LED_DEVICE led);
void led_hw_initialize_leds();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus