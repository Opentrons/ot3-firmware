#pragma once

#if __has_include("stm32l5xx_hal_conf.h")
#include "stm32l5xx_hal_conf.h"
#else
#include "stm32g4xx_hal_conf.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

I2C_HandleTypeDef* MX_I2C_Init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
