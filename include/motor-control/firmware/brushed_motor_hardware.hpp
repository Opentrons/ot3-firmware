#pragma once

#include <cstdint>

#include "motor-control/core/motor_hardware_interface.hpp"

namespace brushed_motor_hardware {

struct PinConfig {
    void* port;
    uint16_t pin;
    uint8_t active_setting;
};

struct HardwareConfig {
    PinConfig pwm_1;
    PinConfig pwm_2;
    PinConfig enable;
    PinConfig limit_switch;
};

class BrushedMotorHardware : public MotorHardwareIface {
  public:
    ~MotorHardware() final = default;
    MotorHardware() = delete;
    MotorHardware(const HardwareConfig& config, void* dac_handle)
        : pins(config), dac_handle(dac_handle) {}
    MotorHardware(const MotorHardware&) = default;
    auto operator=(const MotorHardware&) -> MotorHardware& = default;
    MotorHardware(MotorHardware&&) = default;
    auto operator=(MotorHardware&&) -> MotorHardware& = default;
    void positive_direction() final;
    void negative_direction() final;
    void activate_motor() final;
    void deactivate_motor() final;
    void set_vref() final;
    void stop_dac_interrupt() final;
    auto check_limit_switch() -> bool final;
    void set_LED(bool status) final;

  private:
    HardwareConfig pins;
    void* dac_handle;
};

};  // namespace brushed_motor_hardware
