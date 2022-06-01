#pragma once

#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace hardware {
class SimulatedSensorHardware : public SensorHardwareBase {
  public:
    auto set_sync_out() -> void override {}
    auto reset_sync_out() -> void override {}
    auto check_data_ready() -> void override {}
};
};  // namespace hardware
};  // namespace sensors
