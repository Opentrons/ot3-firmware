#include "common/firmware/adc.h"

#include <stdbool.h>

#include "common/firmware/errors.h"

ADC_HandleTypeDef adc1;
ADC_HandleTypeDef adc2;

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

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
void MX_ADC1_Init(ADC_HandleTypeDef* adc1) {
    /** Common config
     */
    adc1->Instance = ADC1;
    adc1->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
    adc1->Init.Resolution = ADC_RESOLUTION_12B;
    adc1->Init.ScanConvMode = ADC_SCAN_ENABLE;
    adc1->Init.ContinuousConvMode = DISABLE;
    adc1->Init.DiscontinuousConvMode = DISABLE;
    adc1->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc1->Init.NbrOfConversion = 1;
    adc1->Init.DMAContinuousRequests = DISABLE;
    adc1->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc1->Init.LowPowerAutoWait = DISABLE;
    adc1->Init.Overrun = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(adc1) != HAL_OK) {
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
    adc2->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
    adc2->Init.Resolution = ADC_RESOLUTION_12B;
    adc2->Init.ScanConvMode = ADC_SCAN_ENABLE;
    adc2->Init.ContinuousConvMode = DISABLE;
    adc2->Init.DiscontinuousConvMode = DISABLE;
    adc2->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc2->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc2->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc2->Init.NbrOfConversion = 2;
    adc2->Init.DMAContinuousRequests = DISABLE;
    adc2->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc2->Init.LowPowerAutoWait = DISABLE;
    adc2->Init.Overrun = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(adc2) != HAL_OK) {
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

void ADC_set_chan(uint32_t chan, ADC_HandleTypeDef* handle) {
    ADC_ChannelConfTypeDef sConfig = {0};
    /** Configure for the selected ADC regular channel its corresponding rank in
     * the sequencer and its sample time.
     */
    sConfig.Channel = chan;
    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
    if (HAL_ADC_ConfigChannel(handle, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

void adc_read_voltages() {
    uint32_t adc1_value = 4294967295;
    uint32_t adc2_value = 4294967295;
    uint32_t adc3_value = 4294967295;

    ADC_set_chan(ADC_CHANNEL_12, &adc1);
    HAL_ADC_Start(&adc1);
    HAL_ADC_PollForConversion(&adc1, HAL_MAX_DELAY);
    adc1_value = HAL_ADC_GetValue(&adc1);

    ADC_set_chan(ADC_CHANNEL_12, &adc2);
    HAL_ADC_Start(&adc2);
    HAL_ADC_PollForConversion(&adc2, HAL_MAX_DELAY);
    adc2_value = HAL_ADC_GetValue(&adc2);

    ADC_set_chan(ADC_CHANNEL_11, &adc2);
    HAL_ADC_Start(&adc2);
    HAL_ADC_PollForConversion(&adc2, HAL_MAX_DELAY);
    adc3_value = HAL_ADC_GetValue(&adc2);

    struct voltage_read voltage_read = {
        .z_motor = adc3_value, .a_motor = adc2_value, .gripper = adc1_value};

    if ((voltage_read.a_motor == voltage_read.z_motor) ==
        (voltage_read.gripper == 0)) {
        struct states states = {.z_pipette_state = disconnected,
                                .a_pipette_state = disconnected,
                                .gripper_state = disconnected};
        if ((states.a_pipette_state == states.z_pipette_state) ==
            states.gripper_state) {
        }
    }

    else {
    }
}
