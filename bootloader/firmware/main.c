#include "platform_specific_hal_conf.h"
#include "common/firmware/can.h"
#include "common/firmware/errors.h"
#include "bootloader/core/message_handler.h"
#include "can/firmware/utils.h"

/**
 * The CAN handle.
 */
static FDCAN_HandleTypeDef hcan1;

/**
 * Send header
 */
static FDCAN_TxHeaderTypeDef tx_header;

/**
 * Receive header
 */
static FDCAN_RxHeaderTypeDef rx_header;

static Message rx_message;
static Message tx_message;


int main() {
    if (MX_FDCAN1_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_FDCAN_Start(&hcan1) != HAL_OK) {
        Error_Handler();
    }

    tx_header.IdType = FDCAN_EXTENDED_ID;
    tx_header.TxFrameType = FDCAN_DATA_FRAME;
    tx_header.DataLength = FDCAN_DLC_BYTES_8;
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    tx_header.FDFormat = FDCAN_FD_CAN;
    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker = 0;

    for (;;) {
        if (HAL_FDCAN_GetRxFifoFillLevel(&hcan1, FDCAN_RX_FIFO0) > 0) {

            if (HAL_FDCAN_GetRxMessage(&hcan1, FDCAN_RX_FIFO0, &rx_header,
                                   rx_message.data) != HAL_OK) {
                // Read request Error
                Error_Handler();
            }

            rx_message.arbitration_id.id = rx_header.Identifier;
            rx_message.size = length_from_hal(rx_header.DataLength);

            HandleMessageReturn return_code = handle_message(&rx_message, &tx_message);

            if (return_code == handle_message_has_response) {
                tx_header.Identifier = tx_message.arbitration_id.id;
                tx_header.DataLength = length_to_hal(tx_message.size);

                if (HAL_FDCAN_AddMessageToTxFifoQ(&hcan1, &tx_header,
                                                  tx_message.data) != HAL_OK) {
                    // Transmission request Error
                    Error_Handler();
                }
            }
        }
    }
}
