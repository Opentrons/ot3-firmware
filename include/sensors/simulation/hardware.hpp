#pragma once

#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace hardware {
class SimulatedSensorHardware : public SensorHardwareBase {
  public:
    auto set_sync() -> void override {}
    auto reset_sync() -> void override {}
    auto check_data_ready() -> bool override { return data_ready; }
    auto change_data_ready_val(bool val) -> void { data_ready = val; }

  private:
    bool data_ready = false;
};
};  // namespace hardware
};  // namespace sensors
