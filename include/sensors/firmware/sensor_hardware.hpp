#pragma once

#include "common/firmware/gpio.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensor_hardware {
class SensorHardware : public SensorHardwareBase {
  public:
    explicit SensorHardware(gpio::PinConfig sync) : sync_pin(sync) {}
    auto set_sync() -> void override { gpio::set(sync_pin); }
    auto reset_sync() -> void override { gpio::reset(sync_pin); }
    gpio::PinConfig sync_pin;
};
};  // namespace sensor_hardware
