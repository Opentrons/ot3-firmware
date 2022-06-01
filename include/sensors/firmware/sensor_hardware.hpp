#pragma once

#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace hardware {

class SensorHardware : public SensorHardwareBase {
  public:
    SensorHardware(SensorHardwareConfiguration hardware) : hardware(hardware) {}
    auto set_sync() -> void override;
    auto reset_sync() -> void override;
    auto check_data_ready() -> bool override;
    SensorHardwareConfiguration hardware;
};

};  // namespace hardware
};  // namespace sensors
