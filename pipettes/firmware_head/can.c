#include "common/firmware/can.h"

/**
 * Initialize a connection to FDCAN1
 *
 * @param handle Pointer to an FDCAN handle
 * @return HAL_OK on success
 */
HAL_StatusTypeDef MX_FDCAN1_Init(
    FDCAN_HandleTypeDef* handle,
    uint8_t clock_divider,
    uint8_t segment_1_tqs,
    uint8_t segment_2_tqs,
    uint8_t max_sync_jump_width) {
    handle->Instance = FDCAN1;
    handle->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    handle->Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
    handle->Init.Mode = FDCAN_MODE_NORMAL;
    handle->Init.AutoRetransmission = ENABLE;
    handle->Init.TransmitPause = DISABLE;
    handle->Init.ProtocolException = DISABLE;
    handle->Init.NominalPrescaler = clock_divider;
    handle->Init.NominalSyncJumpWidth = max_sync_jump_width;
    handle->Init.NominalTimeSeg1 = segment_1_tqs;
    handle->Init.NominalTimeSeg2 = segment_2_tqs;
    handle->Init.DataPrescaler = clock_divider;
    handle->Init.DataSyncJumpWidth = max_sync_jump_width;
    handle->Init.DataTimeSeg1 = segment_1_tqs;
    handle->Init.DataTimeSeg2 = segment_2_tqs;
    handle->Init.StdFiltersNbr = 20;
    handle->Init.ExtFiltersNbr = 20;
    handle->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    return HAL_FDCAN_Init(handle);
}


/**
 * @brief FDCAN MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hfdcan: FDCAN handle pointer
 * @retval None
 */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hfdcan->Instance == FDCAN1) {
        /* Peripheral clock enable */
        __HAL_RCC_FDCAN_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**FDCAN1 GPIO Configuration
        PA11     ------> FDCAN1_RX
        PA12     ------> FDCAN1_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // Priority is 0-15 (highest to lowest). Use lowest priority until we
        // believe it is too low.
        HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 15, 15);
        HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    }
}

/**
 * @brief FDCAN MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hfdcan: FDCAN handle pointer
 * @retval None
 */
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan) {
    if (hfdcan->Instance == FDCAN1) {
        /* Peripheral clock disable */
        __HAL_RCC_FDCAN_CLK_DISABLE();

        /**FDCAN1 GPIO Configuration
        PA11     ------> FDCAN1_RX
        PA12     ------> FDCAN1_TX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
    }
}
