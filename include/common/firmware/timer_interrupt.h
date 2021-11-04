#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/*
 * Whenever a timer interrupt is defined, the following functions should be
 * included in the `.c`.
 *
 * timer_init -> setups up any GPIOs and configurations needed for that
 * particular interrupt timer. timer_interrupt_start -> calls
 * HAL_TIM_Base_Start_IT timer_interrupt_stop -> calls HAL_TIM_Base_Stop_IT
 */
void timer_init();
void timer_interrupt_start();
void timer_interrupt_stop();
void step_motor();
void turn_on_step_pin();
void turn_off_step_pin();
void turn_on_direction_pin();
void turn_off_direction_pin();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
