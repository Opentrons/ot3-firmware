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
    PinConfig sync_in;
};

class BrushedMotorHardware : public BrushedMotorHardwareIface {
  public:
    ~BrushedMotorHardware() final = default;
    BrushedMotorHardware() = delete;
    BrushedMotorHardware(const BrushedHardwareConfig& config,
                         void* encoder_handle)
        : pins(config), enc_handle(encoder_handle) {}
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
    void grip() final;
    void home() final;
    auto check_sync_in() -> bool final;
    auto get_encoder_pulses() -> uint32_t final;
    void reset_encoder_pulses() final;

  private:
    BrushedHardwareConfig pins;
    void* enc_handle;
};

};  // namespace motor_hardware
