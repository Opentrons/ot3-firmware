#pragma once
#include <array>
#include <functional>

#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace hardware {

class SensorHardware : public SensorHardwareBase {
  public:
    SensorHardware(sensors::hardware::SensorHardwareConfiguration hardware)
        : hardware(hardware) {}
    auto set_sync() -> void override { gpio::set(hardware.sync_out); }
    auto reset_sync() -> void override { gpio::reset(hardware.sync_out); }
    auto check_tip_presence() -> bool override {
        if (hardware.tip_sense.has_value()) {
            return gpio::is_set(hardware.tip_sense.value());
        }
        return 0;
    }
    // TODO: change data_ready's input parameter to an std::map object, so that
    //  the function being called is definitely the callback corresponding to
    //  the correct GPIO interrupt
    auto data_ready() -> void {
        for (auto &callback_function : data_ready_callbacks) {
            if (callback_function) {
                callback_function();
            }
        }
    }

    sensors::hardware::SensorHardwareConfiguration hardware;

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
};
};  // namespace hardware
};  // namespace sensors
