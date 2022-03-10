#pragma once

#include <stdint.h>

#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim7;
extern DAC_HandleTypeDef hdac1;

typedef void (*motor_interrupt_callback)();

HAL_StatusTypeDef initialize_spi();

void initialize_timer(motor_interrupt_callback callback);

void initialize_dac();

HAL_StatusTypeDef start_dac();

HAL_StatusTypeDef set_dac_value(uint32_t val);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
