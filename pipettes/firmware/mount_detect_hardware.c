#include "mount_detect_hardware.h"
#include "platform_specific_hal_conf.h"
#include "pipettes/core/pipette_type.h"

#include "common/firmware/errors.h"

static ADC_HandleTypeDef hadc1 = {0};

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
void MX_ADC1_Init(ADC_HandleTypeDef* adc1) {
    /** Common config
     */
    adc1->Instance = ADC1;
    adc1->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    adc1->Init.Resolution = ADC_RESOLUTION_12B;
    adc1->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc1->Init.ScanConvMode = ADC_SCAN_DISABLE;
    adc1->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc1->Init.LowPowerAutoWait = DISABLE;
    adc1->Init.ContinuousConvMode = DISABLE;
    adc1->Init.NbrOfConversion = 1;
    adc1->Init.DiscontinuousConvMode = DISABLE;
    adc1->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc1->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc1->Init.DMAContinuousRequests = DISABLE;
    adc1->Init.Overrun = ADC_OVR_DATA_PRESERVED;
    adc1->Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(adc1) != HAL_OK) {
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

        if (get_pipette_type() == NINETY_SIX_CHANNEL) {
            // ADC1_IN9
            __HAL_RCC_GPIOC_CLK_ENABLE();
            GPIO_InitStruct.Pin = GPIO_PIN_3;
            GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        } else {
            __HAL_RCC_GPIOB_CLK_ENABLE();

            GPIO_InitStruct.Pin = GPIO_PIN_0;
            GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        }

    }
}

void adc_init(void) {
    MX_ADC1_Init(&hadc1);
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
        /* Calibration Error */
        Error_Handler();
    }
    ADC_ChannelConfTypeDef sConfig = {0};

    if (get_pipette_type() == NINETY_SIX_CHANNEL) {
        // Configure channel 9 (PC3) for single ended long duration read on
        // the tool ID pin
        // TODO (lc: 10-20-2022) Verify these settings are still OK
        // for channel 9 of the adc.
        sConfig.Channel = ADC_CHANNEL_9;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
        sConfig.SingleDiff = ADC_SINGLE_ENDED;
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        sConfig.Offset = 0;
    } else {
        // Configure channel 15 (PB0) for single ended long duration read on
        // the tool ID pin
        sConfig.Channel = ADC_CHANNEL_15;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
        sConfig.SingleDiff = ADC_SINGLE_ENDED;
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        sConfig.Offset = 0;
    }


    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}


uint16_t adc_read(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc1);
}
