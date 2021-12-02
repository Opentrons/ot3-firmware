#ifndef __ADC_H__
#define __ADC_H__

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Initialize ADC
 *
 * @param handle Pointer to a ADC_HandleTypeDef
 * @return HAL_OK on success
 */
HAL_StatusTypeDef MX_ADC_Init(ADC_HandleTypeDef* handle);
void adc_setup(ADC_HandleTypeDef* handle);

#ifdef __cplusplus
}  // extern "C"
#endif // __cplusplus

#endif  // __ADC_H__