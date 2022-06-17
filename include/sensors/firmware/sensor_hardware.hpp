#pragma once
#include "sensors/core/sensor_hardware_interface.hpp"
#include "sensors/firmware/pressure_sensor_callbacks.h"

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

    sensors::hardware::SensorHardwareConfiguration hardware;


    // dont need
    static bool sensor_hardware_callback() {
        if (data_ready_callback != nullptr) {
            data_ready_callback();
            return true;
        }
        else return false;
    }
};
};  // namespace hardware
};  // namespace sensors
