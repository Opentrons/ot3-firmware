#include "mount_interface.h"
#include "platform_specific_hal_conf.h"

static ADC_HandleTypeDef mount_iface_adc_handle = {0};

static void MX_ADC1_Init(ADC_HandleTypeDef* adc1) {
    /** Common config
     */
    adc1->Instance = ADC1;
    adc1->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    adc1->Init.Resolution = ADC_RESOLUTION_12B;
    adc1->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc1->Init.GainCompensation = 0;
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

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {
        .Pin = GPIO_PIN_1,
        .Mode = GPIO_MODE_ANALOG,
        .Pull = GPIO_NOPULL
    };
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void initialize_mount_interface() {
    MX_ADC1_Init(&mount_iface_adc_handle);
}

uint16_t read_mount_interface_adc() {
    ADC_ChannelConfTypeDef sConfig = {0};
    /** Configure for the selected ADC regular channel its corresponding rank in
     * the sequencer and its sample time.
     */
    sConfig.Channel = ADC_CHANNEL_16;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&mount_iface_adc_handle, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    HAL_ADC_Start(&mount_iface_adc_handle);
    HAL_ADC_PollForConversion(&mount_iface_adc_handle, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&mount_iface_adc_handle);
}
