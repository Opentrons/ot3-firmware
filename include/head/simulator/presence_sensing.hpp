#pragma once

#include "head/core/attached_tools.hpp"
#include "head/core/presence_sensing_driver.hpp"

namespace presence_sensing_driver {
class PresenceSensingSimulator : public PresenceSensingDriver {
  public:
    PresenceSensingSimulator() {}
    auto get_readings() -> attached_tools::MountPinMeasurements override {
        return attached_tools::MountPinMeasurements{.left_present = false,
                                                    .right_present = false,
                                                    .gripper_present = false};
    }
    ~PresenceSensingSimulator() override = default;
};
}  // namespace presence_sensing_driver
