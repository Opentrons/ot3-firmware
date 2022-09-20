#pragma once

#include <stdint.h>

#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"
#include "stm32g4xx_it.h"

// The frequency of one full PWM cycle
#define GRIPPER_JAW_PWM_FREQ_HZ (32000UL)
// the number of selectable points in the PWM
#define GRIPPER_JAW_PWM_WIDTH (100UL)
// the frequency at which the timer should count so that it
// does a full PWM cycle in the time specified by GRIPPER_JAW_PWM_FREQ_HZ
#define GRIPPER_JAW_TIMER_FREQ (GRIPPER_JAW_PWM_FREQ_HZ * GRIPPER_JAW_PWM_WIDTH)

#define GRIPPER_ENCODER_SPEED_TIMER_FREQ (1000000UL)


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
typedef void (*encoder_overflow_callback)(int32_t);

uint32_t round_closest(uint32_t dividend, uint32_t divisor);
uint32_t calc_prescaler(uint32_t timer_clk_freq, uint32_t counter_clk_freq);
void initialize_enc();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
