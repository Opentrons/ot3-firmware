#ifndef __ADC_H__
#define __ADC_H__

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern ADC_HandleTypeDef adc1;
extern ADC_HandleTypeDef adc2;

void MX_ADC1_Init(ADC_HandleTypeDef* adc1);
void MX_ADC2_Init(ADC_HandleTypeDef* adc2);
void ADC_set_chan(uint32_t chan, ADC_HandleTypeDef* handle);
//void adc_setup(ADC_HandleTypeDef adc1, ADC_HandleTypeDef adc2);
void adc_setup();

uint32_t adc_read_voltage_z_motor();
uint32_t adc_read_voltage_a_motor();
uint32_t adc_read_voltage_gripper();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // __ADC_H__