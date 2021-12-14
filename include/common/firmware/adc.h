#ifndef __ADC_H__
#define __ADC_H__

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern ADC_HandleTypeDef adc1;
extern ADC_HandleTypeDef adc2;

typedef enum { connected, disconnected } state;

struct states {
    state z_pipette_state;
    state a_pipette_state;
    state gripper_state;
};

struct voltage_read {
    uint32_t z_motor;
    uint32_t a_motor;
    uint32_t gripper;
};


void MX_ADC1_Init(ADC_HandleTypeDef* adc1);
void MX_ADC2_Init(ADC_HandleTypeDef* adc2);
void ADC_set_chan(uint32_t chan, ADC_HandleTypeDef* handle);
void adc_setup();
void adc_read_voltages();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // __ADC_H__