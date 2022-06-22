#pragma once
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/firmware/pressure_sensor_callbacks.h"
#include <array>
#include <functional>

namespace sensors {
namespace hardware {

class SensorHardware : public SensorHardwareBase {
  public:
    SensorHardware(sensors::hardware::SensorHardwareConfiguration hardware)
        : hardware(hardware) {}
    auto set_sync() -> void override { gpio::set(hardware.sync_out); }
    auto reset_sync() -> void override { gpio::reset(hardware.sync_out); }
    auto check_data_ready() -> bool override {
        return gpio::is_set(hardware.data_ready);
    }

    auto data_ready() -> void {
        for (auto &callback_function : data_ready_callbacks) {
            if (callback_function) {
                callback_function();
            }
        }

    }

    sensors::hardware::SensorHardwareConfiguration hardware;

    std::array<std::function<void()>, 5> data_ready_callbacks = {};

    auto add_data_ready_callback(std::function<void()> callback) -> bool {
        for (auto &callback_function : data_ready_callbacks) {
            if (callback_function) {
                continue;
            }
            callback_function = callback;
            return true;
        }
        return false;
    }
};
};  // namespace hardware
};  // namespace sensors
