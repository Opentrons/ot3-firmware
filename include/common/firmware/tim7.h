#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include "stm32g4xx_hal.h"

void MX_GPIO_Init();
void MX_TIM7_Init();
void TIM7_Init();
void TIM7_Start_IT();
void TIM7_Stop_IT();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
