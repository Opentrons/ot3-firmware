#pragma once
#include "can/core/ids.hpp"
#include "common/core/adc_channel.hpp"
#include "common/core/tool_detection.hpp"

namespace pipette_mounts {
inline auto decide_id(adc::millivolts_t reading) -> can::ids::NodeId {
    if (tool_detection::carrier_from_reading(
            reading,
            lookup_table_filtered(tool_detection::Carrier::A_CARRIER)) ==
        tool_detection::Carrier::A_CARRIER) {
        return can::ids::NodeId::pipette_right;
    }
    return can::ids::NodeId::pipette_left;
}

};  // namespace pipette_mounts
