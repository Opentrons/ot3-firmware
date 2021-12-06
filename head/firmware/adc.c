#include "common/firmware/adc.h"

#include <stdbool.h>

#include "common/firmware/errors.h"

ADC_HandleTypeDef adc1;
ADC_HandleTypeDef adc2;

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
void MX_ADC1_Init(ADC_HandleTypeDef* adc1) {
    ADC_MultiModeTypeDef multimode = {0};
    ADC_InjectionConfTypeDef sConfigInjected = {0};

    /** Common config
     */
    adc1->Instance = ADC1;
    adc1->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
    adc1->Init.Resolution = ADC_RESOLUTION_12B;
    adc1->Init.ScanConvMode = ADC_SCAN_ENABLE;
    adc1->Init.ContinuousConvMode = DISABLE;
    adc1->Init.DiscontinuousConvMode = DISABLE;
    adc1->Init.DataAlign = ADC_DATAALIGN_LEFT;
    adc1->Init.NbrOfConversion = 1;
    adc1->Init.DMAContinuousRequests = DISABLE;
    adc1->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc1->Init.LowPowerAutoWait = DISABLE;
    adc1->Init.Overrun = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(adc1) != HAL_OK) {
        Error_Handler();
    }
    /** Configure the ADC multi-mode
     */
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(adc1, &multimode) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Injected Channel
     */
    sConfigInjected.InjectedChannel = ADC_CHANNEL_12;
    sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
    sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
    sConfigInjected.InjectedNbrOfConversion = 2;
    sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_12CYCLES_5;
    sConfigInjected.ExternalTrigInjecConvEdge =
        ADC_EXTERNALTRIGINJECCONV_EDGE_RISING;
    sConfigInjected.ExternalTrigInjecConv = ADC_EXTERNALTRIGINJEC_T1_TRGO;
    sConfigInjected.AutoInjectedConv = DISABLE;
    sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
    sConfigInjected.QueueInjectedContext = ENABLE;
    sConfigInjected.InjectedOffset = 0;
    sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
    if (HAL_ADCEx_InjectedConfigChannel(adc1, &sConfigInjected) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief ADC2 Initialization Function
 * @param None
 * @retval None
 */
void MX_ADC2_Init(ADC_HandleTypeDef* adc2) {
    ADC_InjectionConfTypeDef sConfigInjected = {0};
    // ADC_ChannelConfTypeDef sConfig = {0};

    /** Common config
     */
    adc2->Instance = ADC2;
    adc2->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
    adc2->Init.Resolution = ADC_RESOLUTION_12B;
    adc2->Init.ScanConvMode = ADC_SCAN_ENABLE;
    adc2->Init.ContinuousConvMode = DISABLE;
    adc2->Init.DiscontinuousConvMode = DISABLE;
    adc2->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc2->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc2->Init.DataAlign = ADC_DATAALIGN_LEFT;
    adc2->Init.NbrOfConversion = 2;
    adc2->Init.DMAContinuousRequests = DISABLE;
    adc2->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc2->Init.LowPowerAutoWait = DISABLE;
    adc2->Init.Overrun = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(adc2) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Injected Channel
     */
    sConfigInjected.InjectedChannel = ADC_CHANNEL_11;
    sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
    sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
    sConfigInjected.InjectedNbrOfConversion = 2;
    sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_12CYCLES_5;
    sConfigInjected.ExternalTrigInjecConvEdge =
        ADC_EXTERNALTRIGINJECCONV_EDGE_RISING;
    sConfigInjected.ExternalTrigInjecConv = ADC_EXTERNALTRIGINJEC_T1_TRGO;
    sConfigInjected.AutoInjectedConv = DISABLE;
    sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
    sConfigInjected.QueueInjectedContext = ENABLE;
    sConfigInjected.InjectedOffset = 0;
    sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
    if (HAL_ADCEx_InjectedConfigChannel(adc2, &sConfigInjected) != HAL_OK) {
        Error_Handler();
    }
    /** Configure Injected Channel
     */
    sConfigInjected.InjectedChannel = ADC_CHANNEL_12;
    sConfigInjected.InjectedRank = ADC_INJECTED_RANK_2;
    if (HAL_ADCEx_InjectedConfigChannel(adc2, &sConfigInjected) != HAL_OK) {
        Error_Handler();
    }
}

static uint32_t HAL_RCC_ADC12_CLK_ENABLED = 0;

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
        HAL_RCC_ADC12_CLK_ENABLED++;
        if (HAL_RCC_ADC12_CLK_ENABLED == 1) {
            __HAL_RCC_ADC12_CLK_ENABLE();
        }

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    } else if (hadc->Instance == ADC2) {
        /* Peripheral clock enable */
        HAL_RCC_ADC12_CLK_ENABLED++;
        if (HAL_RCC_ADC12_CLK_ENABLED == 1) {
            __HAL_RCC_ADC12_CLK_ENABLE();
        }

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

void adc_setup() {
    MX_ADC1_Init(&adc1);
    MX_ADC2_Init(&adc2);
}

void ADC_set_chan(uint32_t chan, uint32_t rank, ADC_HandleTypeDef* handle) {
    ADC_ChannelConfTypeDef sConfig = {0};
    /** Configure for the selected ADC regular channel its corresponding rank in
     * the sequencer and its sample time.
     */
    sConfig.Channel = chan;
    sConfig.Rank = rank;
    sConfig.SamplingTime = ADC_SAMPLETIME_12CYCLES_5;
    if (HAL_ADC_ConfigChannel(handle, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

void adc_read_voltages() {
    uint32_t adc1_value = 555;
    uint32_t adc2_value = 666;
    uint32_t adc3_value = 777;

    ADC_set_chan(ADC_CHANNEL_12, ADC_INJECTED_RANK_1, &adc1);
    HAL_ADC_Start(&adc1);
    HAL_ADC_PollForConversion(&adc1, HAL_MAX_DELAY);
    adc1_value = HAL_ADC_GetValue(&adc1);
    HAL_ADC_Stop(&adc1);

    ADC_set_chan(ADC_CHANNEL_12, ADC_INJECTED_RANK_1, &adc2);
    HAL_ADC_Start(&adc2);
    HAL_ADC_PollForConversion(&adc2, HAL_MAX_DELAY);
    adc2_value = HAL_ADC_GetValue(&adc2);
    HAL_ADC_Stop(&adc2);

    ADC_set_chan(ADC_CHANNEL_11, ADC_INJECTED_RANK_1, &adc2);
    HAL_ADC_Start(&adc2);
    HAL_ADC_PollForConversion(&adc2, HAL_MAX_DELAY);
    adc3_value = HAL_ADC_GetValue(&adc2);
    HAL_ADC_Stop(&adc2);

    struct voltage_read voltage_read = {
        .z_motor = adc3_value, .a_motor = adc2_value, .gripper = adc1_value};

    // bool test_val;
    if ((voltage_read.a_motor == voltage_read.z_motor) ==
        voltage_read.gripper) {
    }

    else {
    }
}
