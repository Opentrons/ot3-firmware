#pragma once

#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace hardware {
class SimulatedSensorHardware : public SensorHardwareBase {
  public:
    auto set_sync() -> void override {}
    auto reset_sync() -> void override {}

    std::array<std::function<void()>, 5> data_ready_callbacks = {};

    auto add_data_ready_callback(std::function<void()> callback)
        -> bool override {
        for (auto &callback_function : data_ready_callbacks) {
            if (callback_function) {
                continue;
            }
            callback_function = callback;
            return true;
        }
        return false;
    }

  private:
};
};  // namespace hardware
};  // namespace sensors
