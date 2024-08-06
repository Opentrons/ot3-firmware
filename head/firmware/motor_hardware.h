#pragma once

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

typedef void (*motor_interrupt_callback)();
typedef void (*encoder_overflow_callback)(int32_t);
typedef void (*diag0_interrupt_callback)();

HAL_StatusTypeDef initialize_spi(SPI_HandleTypeDef* hspi);
void initialize_timer(motor_interrupt_callback callback,
                      encoder_overflow_callback left_enc_overflow_callback,
                      encoder_overflow_callback right_enc_overflow_callback,
                      diag0_interrupt_callback* diag0_z_int_callback,
                      diag0_interrupt_callback* diag0_a_int_callback);
void initialize_rev_specific_pins();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
