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
struct voltage_read adc_read_voltages();
struct voltage_read {
    uint32_t z_motor;
    uint32_t a_motor;
    uint32_t gripper;
};

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // __ADC_H__