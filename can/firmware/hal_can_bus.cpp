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
 * Add an arbitration id filter.
 *
 * @param type the type of filter
 * @param config the filter configuration
 * @param val1 depends on the type. Is either a filter, exact arbitration id, or
 * minimum arbitration id
 * @param val2 depends on the type. Is either a mask, exact arbitration id, or
 * masimum arbitration id
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
 * Send a buffer on can bus
 * @param arbitration_id The arbitration id
 * @param buffer buffer to send
 * @param buffer_length length of buffer
 */
void HalCanBus::send(uint32_t arbitration_id, uint8_t* buffer,
                     CanFDMessageLength buffer_length) {
    tx_header.Identifier = arbitration_id;
    tx_header.DataLength = HalCanBus::convert_length(buffer_length);
    tx_header.MessageMarker = 0;

    HAL_FDCAN_AddMessageToTxFifoQ(handle, &tx_header, buffer);
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
 * Convert a length to the hal encoded length.
 * @param length length
 * @return hal encoded length
 */
auto HalCanBus::convert_length(CanFDMessageLength length) -> uint32_t {
    switch (length) {
        case CanFDMessageLength::l0:
            return FDCAN_DLC_BYTES_0;
        case CanFDMessageLength::l1:
            return FDCAN_DLC_BYTES_1;
        case CanFDMessageLength::l2:
            return FDCAN_DLC_BYTES_2;
        case CanFDMessageLength::l3:
            return FDCAN_DLC_BYTES_3;
        case CanFDMessageLength::l4:
            return FDCAN_DLC_BYTES_4;
        case CanFDMessageLength::l5:
            return FDCAN_DLC_BYTES_5;
        case CanFDMessageLength::l6:
            return FDCAN_DLC_BYTES_6;
        case CanFDMessageLength::l7:
            return FDCAN_DLC_BYTES_7;
        case CanFDMessageLength::l8:
            return FDCAN_DLC_BYTES_8;
        case CanFDMessageLength::l12:
            return FDCAN_DLC_BYTES_12;
        case CanFDMessageLength::l16:
            return FDCAN_DLC_BYTES_16;
        case CanFDMessageLength::l20:
            return FDCAN_DLC_BYTES_20;
        case CanFDMessageLength::l24:
            return FDCAN_DLC_BYTES_24;
        case CanFDMessageLength::l32:
            return FDCAN_DLC_BYTES_32;
        case CanFDMessageLength::l48:
            return FDCAN_DLC_BYTES_48;
        case CanFDMessageLength::l64:
            return FDCAN_DLC_BYTES_64;
        default:
            return FDCAN_DLC_BYTES_64;
    }
}

/**
 * Convert a hal encoded length to a CanFDMessageLength
 * @param length hal encoded length
 * @return CanFDMessageLength
 */
auto HalCanBus::convert_length(uint32_t length) -> CanFDMessageLength {
    switch (length) {
        case FDCAN_DLC_BYTES_0: return CanFDMessageLength::l0;
        case FDCAN_DLC_BYTES_1: return CanFDMessageLength::l1;
        case FDCAN_DLC_BYTES_2: return CanFDMessageLength::l2;
        case FDCAN_DLC_BYTES_3: return CanFDMessageLength::l3;
        case FDCAN_DLC_BYTES_4: return CanFDMessageLength::l4;
        case FDCAN_DLC_BYTES_5: return CanFDMessageLength::l5;
        case FDCAN_DLC_BYTES_6: return CanFDMessageLength::l6;
        case FDCAN_DLC_BYTES_7: return CanFDMessageLength::l7;
        case FDCAN_DLC_BYTES_8: return CanFDMessageLength::l8;
        case FDCAN_DLC_BYTES_12: return CanFDMessageLength::l12;
        case FDCAN_DLC_BYTES_16: return CanFDMessageLength::l16;
        case FDCAN_DLC_BYTES_20: return CanFDMessageLength::l20;
        case FDCAN_DLC_BYTES_24: return CanFDMessageLength::l24;
        case FDCAN_DLC_BYTES_32: return CanFDMessageLength::l32;
        case FDCAN_DLC_BYTES_48: return CanFDMessageLength::l48;
        case FDCAN_DLC_BYTES_64: return CanFDMessageLength::l64;
    }
    return CanFDMessageLength::l64;
}