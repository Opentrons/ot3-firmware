#pragma once

#include <concepts>
#include <cstdint>

#include "can/core/parse.hpp"

namespace can_bus {

enum class CanFilterConfig {
    reject,
    to_fifo0,
    to_fifo1,
    to_fifo0_high_priority,
    to_fifo1_high_priority,
};

enum class CanFilterType {
    /* if the arbitration id matches either of the two values. */
    exact,
    /* if arbitration id is within the range of first and second arguments. */
    range,
    /* if first argument equals the arbitration id masked by second argument. */
    mask,
};

/**
 * Concept describing a Can Bus.
 *
 * @tparam CAN The can bus implementation
 */
template <class CAN, class MESSAGE>
concept CanBus = requires(CAN can, MESSAGE message, uint32_t filter_val,
                          CanFilterType filter_type,
                          CanFilterConfig filter_config) {
    {can.add_filter(filter_type, filter_config, filter_val, filter_val)};
    {can.send(filter_val, message)};
};

}  // namespace can_bus