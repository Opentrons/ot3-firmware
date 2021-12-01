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

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
