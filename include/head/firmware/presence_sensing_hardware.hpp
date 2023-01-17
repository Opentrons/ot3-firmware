#pragma once

#include "common/firmware/gpio.hpp"
#include "head/core/attached_tools.hpp"
#include "head/core/presence_sensing_driver.hpp"

namespace presence_sensing_driver {
class PresenceSensingHardware : public PresenceSensingDriver {
  public:
    PresenceSensingHardware() = delete;
    PresenceSensingHardware(gpio::PinConfig left_mount_id,
                            gpio::PinConfig right_mount_id,
                            gpio::PinConfig gripper_mount_id);
    PresenceSensingHardware(const PresenceSensingHardware&) = default;
    auto operator=(const PresenceSensingHardware&)
        -> PresenceSensingHardware& = default;
    PresenceSensingHardware(PresenceSensingHardware&&) = default;
    auto operator=(PresenceSensingHardware&&)
        -> PresenceSensingHardware& = default;
    auto get_readings() -> attached_tools::MountPinMeasurements override;
    ~PresenceSensingHardware() override = default;
    gpio::PinConfig left_mount;
    gpio::PinConfig right_mount;
    gpio::PinConfig gripper_mount;
};
};  // namespace presence_sensing_driver
