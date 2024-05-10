#pragma once

#include "gantry/core/axis_type.h"
#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim2;

typedef void (*motor_interrupt_callback)();
typedef void (*encoder_overflow_callback)(int32_t);

HAL_StatusTypeDef initialize_spi(enum GantryAxisType);
void gantry_driver_CLK_init(enum GantryAxisType);

void initialize_timer(motor_interrupt_callback callback, encoder_overflow_callback enc_callback);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
