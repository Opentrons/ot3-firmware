#pragma once

#include <stdint.h>
#include "platform_specific_hal_conf.h"
#include "system_stm32g4xx.h"

// The frequency of one full PWM cycle
#define LED_PWM_FREQ_HZ (2000UL)
// the number of selectable points in the PWM
#define LED_PWM_WIDTH (256UL)
// the frequency at which the timer should count so that it
// does a full PWM cycle in the time specified by LED_PWM_FREQ_HZ
#define LED_TIMER_FREQ (LED_PWM_FREQ_HZ * LED_PWM_WIDTH)


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum LED_TYPE {
	DECK_LED,
	RED_UI_LED,
	GREEN_UI_LED,
	BLUE_UI_LED,
	WHITE_UI_LED
}LED_DEVICE;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim15;

void update_pwm(uint32_t duty_cycle, LED_DEVICE led);
void initialize_leds();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
