#pragma once

#include <stdint.h>

#include "hepa-uv/core/constants.h"

#include "platform_specific_hal_conf.h"

// The frequency for the LED's pwm (2kHz)
#define LED_PWM_FREQ_HZ (2000UL)
// the frequency at which the timer should count so that it
// does a full PWM cycle in the time specified by LED_PWM_FREQ_HZ
#define LED_TIMER_FREQ (LED_PWM_FREQ_HZ * PWM_WIDTH)

// The frequency for the HEPA pwm (25kHz)
#define HEPA_PWM_FREQ_HZ (25000UL)
#define HEPA_TIMER_FREQ (HEPA_PWM_FREQ_HZ * PWM_WIDTH)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim20;

extern TIM_OC_InitTypeDef htim1_sConfigOC;
extern TIM_OC_InitTypeDef htim3_sConfigOC;
extern TIM_OC_InitTypeDef htim8_sConfigOC;
extern TIM_OC_InitTypeDef htim16_sConfigOC;
extern TIM_OC_InitTypeDef htim20_sConfigOC;

void initialize_pwm_hardware();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
