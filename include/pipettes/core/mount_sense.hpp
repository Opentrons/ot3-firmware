#pragma once
#include "can/core/ids.hpp"

namespace pipette_mounts {
inline auto decide_id(bool reading) -> can::ids::NodeId {
    return reading ? can::ids::NodeId::pipette_left
                   : can::ids::NodeId::pipette_right;
}

};  // namespace pipette_mounts
