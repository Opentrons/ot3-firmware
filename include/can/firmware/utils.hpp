#pragma once

#include "can/core/can_bus.hpp"

namespace hal_can_utils {

/**
 * Convert a CanFilterConfig to a hal value
 * @param config can filter config enum class
 * @return hal defined constant
 */
auto filter_config_to_hal(can_bus::CanFilterConfig config) -> uint32_t;

/**
 * Convert a CanFilterType to a hal value
 * @param type can filter type enum class
 * @return hal defined constant
 */
auto filter_type_to_hal(can_bus::CanFilterType type) -> uint32_t;

/**
 * Convert a length to the hal encoded length.
 * @param length length
 * @return hal encoded length
 */
auto length_to_hal(can_bus::CanFDMessageLength length) -> uint32_t;

/**
 * Convert a hal encoded length to a CanFDMessageLength
 * @param length hal encoded length
 * @return CanFDMessageLength
 */
auto length_from_hal(uint32_t length) -> can_bus::CanFDMessageLength;

}  // namespace hal_can_utils
