#include "hepa-uv/firmware/uv_hardware.h"
#include "hepa-uv/firmware/utility_gpio.h"
#include "common/firmware/errors.h"

#include "platform_specific_hal_conf.h"
#include "system_stm32g4xx.h"

ADC_HandleTypeDef hadc1;
ADC_ChannelConfTypeDef hadc1_sConfig = {0};

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hadc->Instance == ADC1) {
        __HAL_RCC_ADC12_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /** ADC1 GPIO Configuration
            PA0     ------> ADC1_IN1
        */
        GPIO_InitStruct.Pin = UV_SNS_MCU_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(UV_SNS_MCU_PORT, &GPIO_InitStruct);
    }
}

static void MX_ADC_Init(ADC_TypeDef* adc, uint32_t channel) {
    ADC_MultiModeTypeDef multimode = {0};
    ADC_ChannelConfTypeDef* hadc_sConfig;
    ADC_HandleTypeDef* hadc;

    if (adc == ADC1) {
        hadc = &hadc1;
        hadc_sConfig = &hadc1_sConfig;
    } else {
        Error_Handler();
        return;
    }

    hadc->Instance = adc;
    hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc->Init.Resolution = ADC_RESOLUTION_12B;
    hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc->Init.GainCompensation = 0;
    hadc->Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc->Init.LowPowerAutoWait = DISABLE;
    hadc->Init.ContinuousConvMode = DISABLE;
    hadc->Init.NbrOfConversion = 1;
    hadc->Init.DiscontinuousConvMode = DISABLE;
    hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc->Init.DMAContinuousRequests = DISABLE;
    hadc->Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc->Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(hadc) != HAL_OK) Error_Handler();

    // Configure the ADC multi-mode
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(hadc, &multimode) != HAL_OK)
    {
        Error_Handler();
    }

    // Configure Regular Channel
    hadc_sConfig->Channel = channel;
    hadc_sConfig->Rank = ADC_REGULAR_RANK_1;
    hadc_sConfig->SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
    hadc_sConfig->SingleDiff = ADC_DIFFERENTIAL_ENDED;
    hadc_sConfig->OffsetNumber = ADC_OFFSET_NONE;
    hadc_sConfig->Offset = 0;
    if (HAL_ADC_ConfigChannel(hadc, hadc_sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

uint32_t get_adc_reading(ADC_HandleTypeDef* hadc, uint32_t channel) {
    // Calibrate the ADC to get more accurate reading
    if (HAL_ADCEx_Calibration_Start(hadc, ADC_DIFFERENTIAL_ENDED) != HAL_OK) Error_Handler();

    // Configure the Channel
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_6CYCLES_5;
    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK) Error_Handler();

    // Start The adc
    if (HAL_ADC_Start(hadc) != HAL_OK) Error_Handler();
    // Wait for convertion and stop adc
    HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
    HAL_ADC_Stop(hadc);
    // return the adc conversion result
    return HAL_ADC_GetValue(hadc);
}

uint32_t get_uv_light_voltage_reading(void) {
    uint32_t adc_reading = get_adc_reading(&hadc1, ADC_CHANNEL_1);
    // mvolts = (ADC Reading * System mV) / 12-bit adc resolution
    return (uint32_t)(adc_reading*3300)/4095;;
}

void initialize_adc_hardware() {
    // Initialize the adc for uv current sense
    MX_ADC_Init(ADC1, ADC_CHANNEL_1);
}