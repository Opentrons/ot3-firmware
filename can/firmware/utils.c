#include "can/firmware/utils.h"

#include "platform_specific_hal_conf.h"

/**
 * Convert a CanFilterConfig to a hal value
 * @param config can filter config enum class
 * @return hal defined constant
 */
uint32_t filter_config_to_hal(enum CanFilterConfig config)  {
    uint32_t result = FDCAN_FILTER_DISABLE;
    switch (config) {
        case reject:
            result = FDCAN_FILTER_REJECT;
            break;
        case to_fifo0:
            result = FDCAN_FILTER_TO_RXFIFO0;
            break;
        case to_fifo1:
            result = FDCAN_FILTER_TO_RXFIFO1;
            break;
        case to_fifo0_high_priority:
            result = FDCAN_FILTER_TO_RXFIFO0_HP;
            break;
        case to_fifo1_high_priority:
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
uint32_t filter_type_to_hal(enum CanFilterType type)  {
    uint32_t result = FDCAN_FILTER_RANGE;
    switch (type) {
        case exact:
            result = FDCAN_FILTER_DUAL;
            break;
        case range:
            result = FDCAN_FILTER_RANGE;
            break;
        case mask:
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
uint32_t length_to_hal(enum CanFDMessageLength length)  {
    switch (length) {
        case l0:
            return FDCAN_DLC_BYTES_0;
        case l1:
            return FDCAN_DLC_BYTES_1;
        case l2:
            return FDCAN_DLC_BYTES_2;
        case l3:
            return FDCAN_DLC_BYTES_3;
        case l4:
            return FDCAN_DLC_BYTES_4;
        case l5:
            return FDCAN_DLC_BYTES_5;
        case l6:
            return FDCAN_DLC_BYTES_6;
        case l7:
            return FDCAN_DLC_BYTES_7;
        case l8:
            return FDCAN_DLC_BYTES_8;
        case l12:
            return FDCAN_DLC_BYTES_12;
        case l16:
            return FDCAN_DLC_BYTES_16;
        case l20:
            return FDCAN_DLC_BYTES_20;
        case l24:
            return FDCAN_DLC_BYTES_24;
        case l32:
            return FDCAN_DLC_BYTES_32;
        case l48:
            return FDCAN_DLC_BYTES_48;
        case l64:
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
enum CanFDMessageLength length_from_hal(uint32_t length)  {
    switch (length) {
        case FDCAN_DLC_BYTES_0:
            return l0;
        case FDCAN_DLC_BYTES_1:
            return l1;
        case FDCAN_DLC_BYTES_2:
            return l2;
        case FDCAN_DLC_BYTES_3:
            return l3;
        case FDCAN_DLC_BYTES_4:
            return l4;
        case FDCAN_DLC_BYTES_5:
            return l5;
        case FDCAN_DLC_BYTES_6:
            return l6;
        case FDCAN_DLC_BYTES_7:
            return l7;
        case FDCAN_DLC_BYTES_8:
            return l8;
        case FDCAN_DLC_BYTES_12:
            return l12;
        case FDCAN_DLC_BYTES_16:
            return l16;
        case FDCAN_DLC_BYTES_20:
            return l20;
        case FDCAN_DLC_BYTES_24:
            return l24;
        case FDCAN_DLC_BYTES_32:
            return l32;
        case FDCAN_DLC_BYTES_48:
            return l48;
        case FDCAN_DLC_BYTES_64:
            return l64;
    }
    return l64;
}
