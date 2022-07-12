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

void initialize_enc();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
