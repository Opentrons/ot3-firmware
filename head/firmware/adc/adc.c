#include "head/firmware/adc.h"

#include <stdbool.h>

#include "common/firmware/errors.h"

ADC_HandleTypeDef adc1 = {0};
ADC_HandleTypeDef adc2 = {0};

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
void MX_ADC1_Init(ADC_HandleTypeDef* adc) {
    /** Common config
     */
    adc->Instance = ADC1;
    adc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    adc->Init.Resolution = ADC_RESOLUTION_12B;
    adc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc->Init.GainCompensation = 0;
    adc->Init.ScanConvMode = ADC_SCAN_DISABLE;
    adc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc->Init.LowPowerAutoWait = DISABLE;
    adc->Init.ContinuousConvMode = DISABLE;
    adc->Init.NbrOfConversion = 1;
    adc->Init.DiscontinuousConvMode = DISABLE;
    adc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc->Init.DMAContinuousRequests = DISABLE;
    adc->Init.Overrun = ADC_OVR_DATA_PRESERVED;
    adc->Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(adc) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief ADC2 Initialization Function
 * @param None
 * @retval None
 */
void MX_ADC2_Init(ADC_HandleTypeDef* adc2) {
    /** Common config
     */

    adc2->Instance = ADC2;
    adc2->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    adc2->Init.Resolution = ADC_RESOLUTION_12B;
    adc2->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc2->Init.GainCompensation = 0;
    adc2->Init.ScanConvMode = ADC_SCAN_DISABLE;
    adc2->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc2->Init.LowPowerAutoWait = DISABLE;
    adc2->Init.ContinuousConvMode = DISABLE;
    adc2->Init.NbrOfConversion = 1;
    adc2->Init.DiscontinuousConvMode = DISABLE;
    adc2->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc2->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc2->Init.DMAContinuousRequests = DISABLE;
    adc2->Init.Overrun = ADC_OVR_DATA_PRESERVED;
    adc2->Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(adc2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief ADC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hadc->Instance == ADC1) {
        /* Peripheral clock enable */
        __HAL_RCC_ADC12_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    } else if (hadc->Instance == ADC2) {
        /* Peripheral clock enable */
        __HAL_RCC_ADC12_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void adc_setup(ADC_HandleTypeDef* hadc1, ADC_HandleTypeDef* hadc2) {
    MX_ADC1_Init(hadc1);
    MX_ADC2_Init(hadc2);
    if (HAL_ADCEx_Calibration_Start(hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
        /* Calibration Error */
        Error_Handler();
    }
    if (HAL_ADCEx_Calibration_Start(hadc2, ADC_SINGLE_ENDED) != HAL_OK) {
        /* Calibration Error */
        Error_Handler();
    }
}

void adc_set_chan(uint32_t chan, ADC_HandleTypeDef* handle) {
    ADC_ChannelConfTypeDef sConfig = {0};
    /** Configure for the selected ADC regular channel its corresponding rank in
     * the sequencer and its sample time.
     */
    sConfig.Channel = chan;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(handle, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

uint32_t adc_read(ADC_HandleTypeDef* adc_handle, uint32_t adc_channel) {
    uint32_t return_value = 0;

    adc_set_chan(adc_channel, adc_handle);
    HAL_ADC_Start(adc_handle);
    HAL_ADC_PollForConversion(adc_handle, HAL_MAX_DELAY);
    return_value = HAL_ADC_GetValue(adc_handle);

    // HAL_ADC_Stop(&adc2);
    return return_value;
}
