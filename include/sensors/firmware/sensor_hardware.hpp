#pragma once

#include "common/firmware/gpio.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace hardware {
class SensorHardware : public SensorHardwareBase {
  public:
    explicit SensorHardware(gpio::PinConfig sync, gpio::PinConfig data_ready)
        : sync_pin(sync), data_ready_pin(data_ready) {}
    auto set_sync() -> void override { gpio::set(sync_pin); }
    auto reset_sync() -> void override { gpio::reset(sync_pin); }
    auto check_data_ready() -> void override { gpio::is_set(data_ready_pin); }
    gpio::PinConfig sync_pin;
    gpio::PinConfig data_ready_pin;
};
};  // namespace hardware
};  // namespace sensors
