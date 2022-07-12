#include <stdbool.h>
#include <stdint.h>
#include "bootloader/core/node_id.h"
#include "stm32g4xx_hal.h"

#if defined(node_id_pipette_dynamic)

#define ADC_MAX_VAL 4095
#define ADC_REF_MV  3300
static CANNodeId dynamic_id_backing = can_nodeid_broadcast;
static ADC_HandleTypeDef hadc;

static bool MX_ADC1_Init(void) {
    /** Common config
     */
    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.LowPowerAutoWait = DISABLE;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.NbrOfConversion = 1;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc.Init.OversamplingMode = DISABLE;
    return HAL_OK == HAL_ADC_Init(&hadc);
}


/**
 * @brief ADC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* which_adc) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (which_adc->Instance == ADC1) {
        /* Peripheral clock enable */
        __HAL_RCC_ADC12_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

static bool adc_init(void) {
    if (!MX_ADC1_Init()) {
        return false;
    }
    if (HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED) != HAL_OK) {
        /* Calibration Error */
        return false;
    }
    ADC_ChannelConfTypeDef sConfig = {0};
    // Configure channel 16 (PB1) for single ended long duration read on
    // the tool ID pin
    sConfig.Channel = ADC_CHANNEL_16;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    return HAL_OK == HAL_ADC_ConfigChannel(&hadc, &sConfig);
}


static uint16_t adc_read(void) {
    HAL_ADC_Start(&hadc);
    HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc);
}

static uint16_t adc_convert(uint16_t adc_raw_reading) {
    return ((((uint32_t)adc_raw_reading) * ADC_REF_MV) / ADC_MAX_VAL);
}


static CANNodeId update_dynamic_nodeid() {
    if (!adc_init()) {
        return can_nodeid_pipette_left_bootloader;
    }
    return determine_pipette_node_id(
        adc_convert(adc_read()));
}

static CANNodeId get_dynamic_nodeid() {
    if (dynamic_id_backing == can_nodeid_broadcast) {
        dynamic_id_backing = update_dynamic_nodeid();
    }
    return dynamic_id_backing;
}


#endif // #if defined(node_id_pipette_dynamic)

/**
 * Get the node id this bootloader is installed on
 * @return Node id.
 */
CANNodeId get_node_id(void) {
#if defined node_id_head
    return can_nodeid_head_bootloader;
#elif defined node_id_gantry_x
    return can_nodeid_gantry_x_bootloader;
#elif defined node_id_gantry_y
    return can_nodeid_gantry_y_bootloader;
#elif defined node_id_gripper
    return can_nodeid_gripper_bootloader;
#elif defined(node_id_pipette_left)
    return can_nodeid_pipette_left_bootloader;
#elif defined(node_id_pipette_right)
    return can_nodeid_pipette_right_bootloader
#elif defined(node_id_pipette_dynamic)
    return get_dynamic_nodeid();
#else
#error "No node id"
#endif
}
