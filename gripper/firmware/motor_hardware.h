#pragma once

#include <stdint.h>

#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"
#include "stm32g4xx_it.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim7;
extern DAC_HandleTypeDef hdac1;

typedef void (*motor_interrupt_callback)();

HAL_StatusTypeDef initialize_spi();

void initialize_timer(motor_interrupt_callback callback);

void initialize_dac();

void update_pwm(uint32_t freq, uint32_t duty_cycle);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
