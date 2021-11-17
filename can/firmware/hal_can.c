#include "FreeRTOS.h"
#include "semphr.h"
#include "platform_specific_hal_conf.h"
#include "can/firmware/hal_can.h"
#include "can/firmware/utils.h"

/**
 * Fifo reading mutex.
 */
static StaticSemaphore_t mutex_data;
static SemaphoreHandle_t mutex;


/**
 * CAN received message header
 */
static FDCAN_RxHeaderTypeDef RxHeader;

/**
 * Buffer for received data.
 */
static uint8_t buff[64];

/**
 * The global can 1 handle.
 */
static FDCAN_HandleTypeDef fdcan1;

/**
 * Incoming message callback.
 */
static can_message_callback message_callback = NULL;

/**
 * Initialize CAN1
 *
 * @param handle handle to the can device
 * @return
 */
static HAL_StatusTypeDef MX_FDCAN1_Init(FDCAN_HandleTypeDef* handle);

/**
 * Start CAN
 */
void can_start() {
    mutex = xSemaphoreCreateMutexStatic(&mutex_data);

    // TODO (al, 2021-11-18): error returns?
    MX_FDCAN1_Init(&fdcan1);

    HAL_FDCAN_ActivateNotification(&fdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO1_NEW_MESSAGE,0);

    HAL_FDCAN_Start(&fdcan1);
}


/**
 *
 * Get CAN device handle.
 * @param device which device
 * @return a handle to a device.
 */
HAL_CAN_HANDLE can_get_device_handle() {
    return &fdcan1;
}


/**
 * Register a callback function to process new messages.
 *
 * This is called from an ISR
 *
 * @param callback a callback function.
 */
void can_register_message_callback(can_message_callback callback) {
    message_callback = callback;
}


/**
 * Send a can message.
 *
 * @param handle handle to can device
 * @param arbitration_id the arbitration id
 * @param buffer pointer to start of buffer
 * @param buffer_length length of buffer as defined by HAL
 */
void can_send_message(HAL_CAN_HANDLE handle, uint32_t arbitration_id, uint8_t* buffer,
                      enum CanFDMessageLength  buffer_length)
{
    FDCAN_TxHeaderTypeDef tx_header = {
        .Identifier=arbitration_id,
        .IdType = FDCAN_EXTENDED_ID,
        .TxFrameType = FDCAN_DATA_FRAME,
        .DataLength = length_to_hal(buffer_length),
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch = FDCAN_BRS_OFF,
        .FDFormat = FDCAN_FD_CAN,
        .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
        .MessageMarker=0
    };
    HAL_FDCAN_AddMessageToTxFifoQ(handle, &tx_header, buffer);
}


/**
* Add a filter for incoming messages.
*
* @param handle handle to can device
* @param index index of the filter
* @param type the filter type (range, mask, exact match).
* @param config what the filter should do
* @param val1 filter value
* @param val2 filter value
*/
void can_add_filter(HAL_CAN_HANDLE handle, uint32_t index, enum CanFilterType type, enum CanFilterConfig config,
                uint32_t val1, uint32_t val2)
{
    FDCAN_FilterTypeDef filter_def = {
        .IdType = FDCAN_EXTENDED_ID,
        .FilterIndex = index,
        .FilterType = filter_type_to_hal(type),
        .FilterConfig = filter_config_to_hal(config),
        .FilterID1 = val1,
        .FilterID2 = val2};
    HAL_FDCAN_ConfigFilter(handle, &filter_def);
}


/**
 * Initialize a connection to FDCAN1
 *
 * @param handle Pointer to an FDCAN handle
 * @return HAL_OK on success
 */
HAL_StatusTypeDef MX_FDCAN1_Init(FDCAN_HandleTypeDef* handle) {
    handle->Instance = FDCAN1;
    handle->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    handle->Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
    handle->Init.Mode = FDCAN_MODE_NORMAL;
    handle->Init.AutoRetransmission = ENABLE;
    handle->Init.TransmitPause = DISABLE;
    handle->Init.ProtocolException = DISABLE;
    handle->Init.NominalPrescaler = 20;
    handle->Init.NominalSyncJumpWidth = 2;
    handle->Init.NominalTimeSeg1 = 14;
    handle->Init.NominalTimeSeg2 = 2;
    handle->Init.DataPrescaler = 20;
    handle->Init.DataSyncJumpWidth = 1;
    handle->Init.DataTimeSeg1 = 14;
    handle->Init.DataTimeSeg2 = 1;
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


/**
 * Read from fifo and write read message to the read_can_message_buffer.
 *
 * @param hfdcan pointer to a can handle
 * @param fifo either FDCAN_RX_FIFO0 or FDCAN_RX_FIFO1
 */
static void try_read_from_fifo(FDCAN_HandleTypeDef *hfdcan, uint32_t fifo) {
    if (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, fifo) > 0) {
        // Grab mutex to protect static data
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreTakeFromISR(mutex, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken != pdFALSE) {
            taskYIELD();
        }

        if (HAL_FDCAN_GetRxMessage(hfdcan, fifo, &RxHeader, buff) == HAL_OK) {
            // Convert the length from HAL to integer
            uint32_t data_length = length_from_hal(RxHeader.DataLength);

            if (message_callback) {
                message_callback(RxHeader.Identifier, buff, data_length);
            }
        }
        
        // Release the mutex
        xSemaphoreGiveFromISR(mutex, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken != pdFALSE) {
            taskYIELD();
        }
    }
}


/**
 * ISR for new message in FIFO 0
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0) {
        try_read_from_fifo(hfdcan, FDCAN_RX_FIFO0);
    }
}


/**
 * ISR for new message in FIFO 1
 */
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo1ITs) {
    if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != 0) {
        try_read_from_fifo(hfdcan, FDCAN_RX_FIFO1);
    }
}