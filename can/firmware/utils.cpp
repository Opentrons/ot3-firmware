#include "can/firmware/utils.hpp"

#include "platform_specific_hal_fdcan.h"

using namespace can_bus;

/**
 * Convert a CanFilterConfig to a hal value
 * @param config can filter config enum class
 * @return hal defined constant
 */
auto hal_can_utils::filter_config_to_hal(CanFilterConfig config) -> uint32_t {
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
auto hal_can_utils::filter_type_to_hal(CanFilterType type) -> uint32_t {
    auto result = FDCAN_FILTER_RANGE;
    switch (type) {
        case CanFilterType::exact:
            result = FDCAN_FILTER_DUAL;
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
auto hal_can_utils::length_to_hal(CanFDMessageLength length) -> uint32_t {
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
auto hal_can_utils::length_from_hal(uint32_t length) -> CanFDMessageLength {
    switch (length) {
        case FDCAN_DLC_BYTES_0:
            return CanFDMessageLength::l0;
        case FDCAN_DLC_BYTES_1:
            return CanFDMessageLength::l1;
        case FDCAN_DLC_BYTES_2:
            return CanFDMessageLength::l2;
        case FDCAN_DLC_BYTES_3:
            return CanFDMessageLength::l3;
        case FDCAN_DLC_BYTES_4:
            return CanFDMessageLength::l4;
        case FDCAN_DLC_BYTES_5:
            return CanFDMessageLength::l5;
        case FDCAN_DLC_BYTES_6:
            return CanFDMessageLength::l6;
        case FDCAN_DLC_BYTES_7:
            return CanFDMessageLength::l7;
        case FDCAN_DLC_BYTES_8:
            return CanFDMessageLength::l8;
        case FDCAN_DLC_BYTES_12:
            return CanFDMessageLength::l12;
        case FDCAN_DLC_BYTES_16:
            return CanFDMessageLength::l16;
        case FDCAN_DLC_BYTES_20:
            return CanFDMessageLength::l20;
        case FDCAN_DLC_BYTES_24:
            return CanFDMessageLength::l24;
        case FDCAN_DLC_BYTES_32:
            return CanFDMessageLength::l32;
        case FDCAN_DLC_BYTES_48:
            return CanFDMessageLength::l48;
        case FDCAN_DLC_BYTES_64:
            return CanFDMessageLength::l64;
    }
    return CanFDMessageLength::l64;
}