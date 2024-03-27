#pragma once

#include <stdint.h>

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern ADC_HandleTypeDef hadc1;
extern ADC_ChannelConfTypeDef hadc1_sConfig;

void initialize_adc_hardware();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
