#include "common/firmware/adc.h"
#include "common/firmware/errors.h"

HAL_StatusTypeDef MX_ADC_Init(ADC_HandleTypeDef* handle) {
    handle->Instance = ADC3;
    handle->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
    handle->Init.Resolution = ADC_RESOLUTION_12B;
    handle->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    handle->Init.ScanConvMode = ADC_SCAN_DISABLE;
    handle->Init.ContinuousConvMode = DISABLE;
    handle->Init.NbrOfConversion = 1;
    handle->Init.DiscontinuousConvMode = ENABLE;
    handle->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    handle->Init.DMAContinuousRequests = DISABLE;
    return HAL_ADC_Init(handle);
}

void adc_setup(ADC_HandleTypeDef* handle){
    if (HAL_OK != HAL_ADC_Init(handle)) {
        Error_Handler();
    }
    HAL_ADCEx_Calibration_Start(handle, ADC_SINGLE_ENDED);
}
