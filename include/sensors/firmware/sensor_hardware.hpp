#pragma once
#include <array>
#include <functional>

#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace hardware {

class SensorHardware : public SensorHardwareBase {
  public:
    SensorHardware(sensors::hardware::SensorHardwareConfiguration hardware, SensorHardwareVersionSingleton& version_wrapper)
        :  SensorHardwareBase(version_wrapper), hardware(hardware){}
    auto set_sync() -> void override { gpio::set(hardware.sync_out); }
    auto reset_sync() -> void override { gpio::reset(hardware.sync_out); }
    auto check_tip_presence() -> bool override {
        if (hardware.tip_sense.has_value()) {
            return gpio::is_set(hardware.tip_sense.value());
        }
        return false;
    }

    sensors::hardware::SensorHardwareConfiguration hardware;
};

};  // namespace hardware
};  // namespace sensors
