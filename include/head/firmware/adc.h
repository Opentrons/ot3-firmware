#ifndef __ADC_H__
#define __ADC_H__

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


ADC_HandleTypeDef* get_adc1_handle(void);
ADC_HandleTypeDef* get_adc2_handle(void);
void adc_setup(ADC_HandleTypeDef* adc1, ADC_HandleTypeDef* adc2);
uint32_t adc_read(ADC_HandleTypeDef* adc_handle, uint32_t adc_channel);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // __ADC_H__
