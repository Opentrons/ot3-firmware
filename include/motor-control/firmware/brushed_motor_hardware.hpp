#pragma once

#include <cstdint>

#include "motor-control/core/motor_hardware_interface.hpp"

namespace brushed_motor_hardware {

struct PinConfig {
    void* port;
    uint16_t pin;
    uint8_t active_setting;
};

struct DacConfig {
    void* dac_handle;
    uint32_t channel;
    uint32_t data_algn;
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
    MotorHardware(const HardwareConfig& config, const DacConfig& dac_config)
        : pins(config), dac(dac_config) {}
    MotorHardware(const MotorHardware&) = default;
    auto operator=(const MotorHardware&) -> MotorHardware& = default;
    MotorHardware(MotorHardware&&) = default;
    auto operator=(MotorHardware&&) -> MotorHardware& = default;
    void positive_direction() final;
    void negative_direction() final;
    void activate_motor() final;
    void deactivate_motor() final;
    auto check_limit_switch() -> bool final;
    auto start_digital_analog_converter() -> bool final;
    auto stop_digital_analog_converter() -> bool final;
    void set_vref() final;

  private:
    HardwareConfig pins;
    void* dac;
};

};  // namespace brushed_motor_hardware
