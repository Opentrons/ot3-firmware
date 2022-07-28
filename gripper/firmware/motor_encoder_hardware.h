#pragma once

#include <stdint.h>

#include "common/firmware/errors.h"
#include "platform_specific_hal_conf.h"
#include "stm32g4xx_it.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern TIM_HandleTypeDef htim2;
typedef void (*encoder_overflow_callback)(int32_t);

uint32_t round_closest(uint32_t dividend, uint32_t divisor);
uint32_t calc_prescaler(uint32_t timer_clk_freq, uint32_t counter_clk_freq);
void initialize_enc();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
