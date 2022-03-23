#pragma once

#include <cstdint>

#include "motor-control/core/motor_hardware_interface.hpp"

namespace motor_hardware {

struct PwmConfig {
    void* tim;
    uint32_t channel;
};

struct BrushedHardwareConfig {
    PwmConfig pwm_1;
    PwmConfig pwm_2;
    PinConfig enable;
    PinConfig limit_switch;
};

class BrushedMotorHardware : public BrushedMotorHardwareIface {
  public:
    ~BrushedMotorHardware() final = default;
    BrushedMotorHardware() = delete;
    BrushedMotorHardware(const BrushedHardwareConfig& config) : pins(config) {}
    BrushedMotorHardware(const BrushedMotorHardware&) = default;
    auto operator=(const BrushedMotorHardware&)
        -> BrushedMotorHardware& = default;
    BrushedMotorHardware(BrushedMotorHardware&&) = default;
    auto operator=(BrushedMotorHardware&&) -> BrushedMotorHardware& = default;
    void positive_direction() final;
    void negative_direction() final;
    void activate_motor() final;
    void deactivate_motor() final;
    auto check_limit_switch() -> bool final;

  private:
    BrushedHardwareConfig pins;
};

};  // namespace motor_hardware
