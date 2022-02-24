#pragma once

#include "common/core/adc_channel.hpp"
#include "mount_detect_hardware.h"
#include "pipettes/core/mount_sense.hpp"

namespace pipette_mounts {
class MountDetectADCChannel : public adc::BaseADCChannel<3300, 4095> {
  protected:
    auto get_reading() -> uint16_t override { return adc_read(); }
};

inline auto detect_id() -> can_ids::NodeId {
    return decide_id(MountDetectADCChannel().get_voltage());
}

};  // namespace pipette_mounts
