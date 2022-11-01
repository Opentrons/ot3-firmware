#pragma once

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;

typedef void (*linear_motor_interrupt_callback)();
typedef void (*gear_motor_interrupt_callback)();
typedef void (*encoder_interrupt_callback)(int32_t);

void initialize_linear_timer(linear_motor_interrupt_callback);
void initialize_gear_timer(gear_motor_interrupt_callback);
void initialize_enc_timer(encoder_interrupt_callback callback);
#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus