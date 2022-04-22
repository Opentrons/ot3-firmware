#pragma once

#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace hardware {
class SimulatedSensorHardware : public SensorHardwareBase {
  public:
    auto set_sync() -> void override {}
    auto reset_sync() -> void override {}
};
};  // namespace hardware
};  // namespace sensors
