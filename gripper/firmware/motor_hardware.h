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
extern uint32_t DAC_CHANNEL_1;
extern uint32_t DAC_ALIGN_12B_R;

typedef void (*motor_interrupt_callback)();

HAL_StatusTypeDef initialize_spi();

void initialize_timer(motor_interrupt_callback callback);

void initialize_dac();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
