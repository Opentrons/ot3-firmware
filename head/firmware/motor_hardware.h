#pragma once

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim7;

typedef void (*motor_interrupt_callback)();
HAL_StatusTypeDef initialize_spi(SPI_HandleTypeDef* hspi);
void initialize_timer(motor_interrupt_callback callback);
void debug_left_start();
void debug_left_stop();
void debug_right_start();
void debug_right_stop();


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
