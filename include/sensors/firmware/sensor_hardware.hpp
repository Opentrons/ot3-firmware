#pragma once

#include "common/firmware/gpio.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace hardware {

struct SensorHardwareConfiguration {
    gpio::PinConfig sync_in;
    gpio::PinConfig sync_out;
    gpio::PinConfig data_ready;
};

class SensorHardware : public SensorHardwareBase {
  public:
    SensorHardware(SensorHardwareConfiguration hardware) : hardware(hardware) {}
    auto set_sync() -> void override { gpio::set(hardware.sync_out); }
    auto reset_sync() -> void override { gpio::reset(hardware.sync_out); }
    auto check_data_ready() -> bool override {
        return gpio::is_set(hardware.data_ready);
    }
    SensorHardwareConfiguration hardware;
};
};  // namespace hardware
};  // namespace sensors
