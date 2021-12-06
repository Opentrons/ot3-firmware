#ifndef __ADC_H__
#define __ADC_H__

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
ADC_HandleTypeDef adc1;
ADC_HandleTypeDef adc2;
void MX_ADC1_Init(ADC_HandleTypeDef* adc1);
void MX_ADC2_Init(ADC_HandleTypeDef* adc2);
void adc_setup();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // __ADC_H__