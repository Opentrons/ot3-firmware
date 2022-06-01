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
    auto set_sync_in() -> void override { gpio::set(hardware.sync_in); }
    auto reset_sync_in() -> void override { gpio::reset(hardware.sync_in); }
    auto check_data_ready() -> void override {
        return gpio::is_set(hardware.data_ready);
    }
    SensorHardwareConfiguration hardware;
};
};  // namespace hardware
};  // namespace sensors
