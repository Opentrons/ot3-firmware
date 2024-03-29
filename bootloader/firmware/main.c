#include "platform_specific_hal_conf.h"
#include "platform_specific_hal.h"
#include "common/core/app_update.h"
#include "common/firmware/errors.h"
#include "common/firmware/clocking.h"
#include "common/firmware/iwdg.h"
#include "can/firmware/utils.h"
#include "bootloader/core/message_handler.h"
#include "bootloader/core/node_id.h"
#include "bootloader/core/updater.h"
#include "bootloader/firmware/system.h"
#include "bootloader/firmware/crc32.h"
#include "bootloader/firmware/can.h"
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

/**
 * Check if application needs to be updated.
 * @return true if application needs to be updated.
 */
static bool requires_an_update();

/**
 * The entry point to the CAN application updater.
 */
static void run_update();

/**
 * Initialize the CAN handle.
 * @param can_handle
 */
static void initialize_can(FDCAN_HandleTypeDef * can_handle);


/**
 * Check against RCC_CSR register. Were we started from:
 *   pin reset
 *   low-power reset
 *   brown-out reset
 */
#define RESET_CHECK_MASK (RCC_CSR_PINRSTF | RCC_CSR_LPWRRSTF | RCC_CSR_BORRSTF)

/**
 * Check against the RCC_CSR. Were we started by the watchdog timer.
 */
#define WATCHDOG_CHECK_MASK (RCC_CSR_IWDGRSTF | RCC_CSR_WWDGRSTF)

/**
 * Are we starting due to power on.
 */
#define IS_POWER_ON_RESET() ((RCC->CSR & RESET_CHECK_MASK) != 0)

/**
 * Are we starting due to watchdog reset.
 */
#define IS_WATCHDOG_RESET() ((RCC->CSR & WATCHDOG_CHECK_MASK) != 0)


int main() {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    MX_IWDG_Init();

    // Enable the flash interrupt.
    HAL_NVIC_SetPriority(FLASH_IRQn, 15, 15);
    HAL_NVIC_EnableIRQ(FLASH_IRQn);
    system_specific_startup();
    bool requires_update = requires_an_update();

    // Clear reset flags. Otherwise, they will persist for the lifetime of
    // the bootloader and the application.
    __HAL_RCC_CLEAR_RESET_FLAGS();

    if (requires_update) {
        // Perform the firmware update.
        run_update();
    } else {
        // Start the application.
        fw_update_start_application();
    }
}


/**
 * Check if application needs to be updated.
 * @return true if application needs to be updated.
 */
bool requires_an_update() {
    // An update is required if:
    // - Restarted by watchdog
    // - The application is not in flash
    // - The bootloader is not powering on from a reset and the application has
    //    requested an update.
    if (IS_WATCHDOG_RESET() ||
        (!IS_POWER_ON_RESET() && is_app_update_requested()) ||
        !is_app_in_flash()) {
        return true;
    }
    return false;
}


/**
 * Entry point of the fw update over CAN.
 */
void run_update() {

    crc32_init();

    initialize_can(&hcan1);

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
                // Set the originating id to our node id
                tx_message.arbitration_id.parts.originating_node_id = get_node_id();
                tx_header.Identifier = tx_message.arbitration_id.id;
                tx_header.DataLength = length_to_hal(tx_message.size);

                // Wait for there to be room.
                while (HAL_FDCAN_GetTxFifoFreeLevel(&hcan1) == 0) {
                    HAL_Delay(1);
                }

                if (HAL_FDCAN_AddMessageToTxFifoQ(&hcan1, &tx_header,
                                                  tx_message.data) != HAL_OK) {
                    // Transmission request Error
                    Error_Handler();
                }
            }
        } else {
            HAL_Delay(1);
        }
        // Refresh the watch dog.
        iwdg_refresh();
    }
}


/**
 * Initialize the CAN bus
 * @param can_handle Handle to can bus
 */
void initialize_can(FDCAN_HandleTypeDef * can_handle) {
    if (MX_FDCAN1_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_FDCAN_Start(&hcan1) != HAL_OK) {
        Error_Handler();
    }

    CANArbitrationId arb_mask;
    CANArbitrationId arb_filter;

    FDCAN_FilterTypeDef filter_def = {
        .IdType = FDCAN_EXTENDED_ID,
        .FilterIndex = 0,
        .FilterType = 0,
        .FilterConfig = 0,
        .FilterID1 = 0,
        .FilterID2 = 0};

    // Create filter to accept messages from host to this node
    arb_mask.id = 0;
    arb_mask.parts.node_id = -1;
    arb_mask.parts.originating_node_id = -1;
    arb_filter.id = 0;
    arb_filter.parts.node_id = get_node_id();
    arb_filter.parts.originating_node_id = can_nodeid_host;

    filter_def.FilterIndex = 0;
    filter_def.FilterType = filter_type_to_hal(mask),
    filter_def.FilterConfig = filter_config_to_hal(to_fifo0),
    filter_def.FilterID1 = arb_filter.id,
    filter_def.FilterID2 = arb_mask.id;
    HAL_FDCAN_ConfigFilter(can_handle, &filter_def);

    // Create filter to accept broadcast DeviceInfoRequest messages
    arb_mask.id = 0;
    arb_mask.parts.node_id = -1;
    arb_mask.parts.message_id = -1;
    arb_filter.id = 0;
    arb_filter.parts.node_id = can_nodeid_broadcast;
    arb_filter.parts.message_id = can_messageid_device_info_request;

    filter_def.FilterIndex = 1;
    filter_def.FilterType = filter_type_to_hal(mask),
    filter_def.FilterConfig = filter_config_to_hal(to_fifo0),
    filter_def.FilterID1 = arb_filter.id,
    filter_def.FilterID2 = arb_mask.id;
    HAL_FDCAN_ConfigFilter(can_handle, &filter_def);

    // Reject everything else
    filter_def.FilterIndex = 2;
    filter_def.FilterType = filter_type_to_hal(mask),
    filter_def.FilterConfig = filter_config_to_hal(reject),
    filter_def.FilterID1 = 0,
    filter_def.FilterID2 = 0;
    HAL_FDCAN_ConfigFilter(can_handle, &filter_def);
}

void __attribute__((weak)) system_specific_startup() {}
