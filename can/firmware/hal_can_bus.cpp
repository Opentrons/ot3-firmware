#include "can/firmware/hal_can_bus.hpp"
#include "can/firmware/utils.hpp"
#include "common/core/synchronization.hpp"

HalCanBus::HalCanBus(FDCAN_HandleTypeDef* handle)
    : handle(handle),
      tx_header{.IdType = arbitration_id_type,
                .TxFrameType = FDCAN_DATA_FRAME,
                .ErrorStateIndicator = FDCAN_ESI_PASSIVE,
                .BitRateSwitch = FDCAN_BRS_OFF,
                .FDFormat = FDCAN_FD_CAN,
                .TxEventFifoControl = FDCAN_NO_TX_EVENTS} {}

void HalCanBus::add_filter(CanFilterType type, CanFilterConfig config,
                           uint32_t val1, uint32_t val2) {
    FDCAN_FilterTypeDef filter_def{
        .IdType = arbitration_id_type,
        .FilterIndex = filter_index++,
        .FilterType = hal_can_utils::filter_type_to_hal(type),
        .FilterConfig = hal_can_utils::filter_config_to_hal(config),
        .FilterID1 = val1,
        .FilterID2 = val2};
    HAL_FDCAN_ConfigFilter(handle, &filter_def);
}

void HalCanBus::send(uint32_t arbitration_id, uint8_t* buffer,
                     CanFDMessageLength buffer_length) {
    auto lock = synchronization::Lock{mutex};

    tx_header.Identifier = arbitration_id;
    tx_header.DataLength = hal_can_utils::length_to_hal(buffer_length);
    tx_header.MessageMarker = 0;

    HAL_FDCAN_AddMessageToTxFifoQ(handle, &tx_header, buffer);
}

auto HalCanBus::start() -> HAL_StatusTypeDef {
    auto result = HAL_FDCAN_ActivateNotification(
        handle, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO1_NEW_MESSAGE,
        0);

    if (result != HAL_OK) {
        return result;
    }

    return HAL_FDCAN_Start(handle);
}