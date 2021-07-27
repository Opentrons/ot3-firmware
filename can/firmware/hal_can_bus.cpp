#include "can/firmware/hal_can_bus.hpp"

/**
 * Constructor
 * @param handle Handle to hal fdcan object.
 */
HalCanBus::HalCanBus(FDCAN_HandleTypeDef* handle)
    : handle(handle),
      tx_header{.IdType = arbitration_id_type,
                .TxFrameType = FDCAN_DATA_FRAME,
                .ErrorStateIndicator = FDCAN_ESI_PASSIVE,
                .BitRateSwitch = FDCAN_BRS_OFF,
                .FDFormat = FDCAN_FD_CAN,
                .TxEventFifoControl = FDCAN_NO_TX_EVENTS} {}

/**
 *
 * @param type
 * @param config
 * @param val1
 * @param val2
 */
void HalCanBus::add_filter(CanFilterType type, CanFilterConfig config,
                           uint32_t val1, uint32_t val2) {
    FDCAN_FilterTypeDef filter_def{
        .IdType = arbitration_id_type,
        .FilterIndex = filter_index++,
        .FilterType = HalCanBus::convert_type(type),
        .FilterConfig = HalCanBus::convert_config(config),
        .FilterID1 = val1,
        .FilterID2 = val2};
    HAL_FDCAN_ConfigFilter(handle, &filter_def);
}

/**
 * Convert a CanFilterConfig to a hal value
 * @param config can filter config enum class
 * @return hal defined constant
 */
auto HalCanBus::convert_config(CanFilterConfig config) -> uint32_t {
    auto result = FDCAN_FILTER_DISABLE;
    switch (config) {
        case CanFilterConfig::reject:
            result = FDCAN_FILTER_REJECT;
            break;
        case CanFilterConfig::to_fifo0:
            result = FDCAN_FILTER_TO_RXFIFO0;
            break;
        case CanFilterConfig::to_fifo1:
            result = FDCAN_FILTER_TO_RXFIFO1;
            break;
        case CanFilterConfig::to_fifo0_high_priority:
            result = FDCAN_FILTER_TO_RXFIFO0_HP;
            break;
        case CanFilterConfig::to_fifo1_high_priority:
            result = FDCAN_FILTER_TO_RXFIFO1_HP;
            break;
        default:
            break;
    }
    return result;
}

/**
 * Convert a CanFilterType to a hal value
 * @param type can filter type enum class
 * @return hal defined constant
 */
auto HalCanBus::convert_type(CanFilterType type) -> uint32_t {
    auto result = FDCAN_FILTER_RANGE;
    switch (type) {
        case CanFilterType::exact:
            result = FDCAN_FILTER_RANGE;
            break;
        case CanFilterType::range:
            result = FDCAN_FILTER_RANGE;
            break;
        case CanFilterType::mask:
            result = FDCAN_FILTER_MASK;
            break;
        default:
            break;
    }
    return result;
}

/**
 *
 * @param size
 * @return
 */
auto HalCanBus::convert_size(uint32_t size) -> uint32_t {
    switch (size) {
        case 0:
            return FDCAN_DLC_BYTES_0;
        case 1:
            return FDCAN_DLC_BYTES_1;
        case 2:
            return FDCAN_DLC_BYTES_2;
        case 3:
            return FDCAN_DLC_BYTES_3;
        case 4:
            return FDCAN_DLC_BYTES_4;
        case 5:
            return FDCAN_DLC_BYTES_5;
        case 6:
            return FDCAN_DLC_BYTES_6;
        case 7:
            return FDCAN_DLC_BYTES_7;
        case 8:
            return FDCAN_DLC_BYTES_8;
        default:
            break;
    }
    if (size < 12)
        return FDCAN_DLC_BYTES_12;
    else if (size < 16)
        return FDCAN_DLC_BYTES_16;
    else if (size < 20)
        return FDCAN_DLC_BYTES_20;
    else if (size < 24)
        return FDCAN_DLC_BYTES_24;
    else if (size < 32)
        return FDCAN_DLC_BYTES_32;
    else if (size < 48)
        return FDCAN_DLC_BYTES_48;
    else
        return FDCAN_DLC_BYTES_64;
}