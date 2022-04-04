#pragma once

#include <cstdint>

#include "motor-control/core/motor_hardware_interface.hpp"

namespace motor_hardware {

struct HardwareConfig {
    PinConfig direction;
    PinConfig step;
    PinConfig enable;
    PinConfig limit_switch;
    PinConfig led;
    PinConfig sync_in;
};

class MotorHardware : public StepperMotorHardwareIface {
  public:
    ~MotorHardware() final = default;
    MotorHardware() = delete;
    MotorHardware(const HardwareConfig& config, void* timer_handle, void* encoder_handle)
        : pins(config), tim_handle(timer_handle) , enc_handle(encoder_handle){}
    MotorHardware(const MotorHardware&) = default;
    auto operator=(const MotorHardware&) -> MotorHardware& = default;
    MotorHardware(MotorHardware&&) = default;
    auto operator=(MotorHardware&&) -> MotorHardware& = default;
    void step() final;
    void unstep() final;
    void positive_direction() final;
    void negative_direction() final;
    void activate_motor() final;
    void deactivate_motor() final;
    void start_timer_interrupt() final;
    void stop_timer_interrupt() final;
    auto check_limit_switch() -> bool final;
    void set_LED(bool status) final;
    auto check_sync_in() -> bool final;
    uint32_t get_encoder_pulses() final;
    void reset_encoder_pulses() final;

  private:
    HardwareConfig pins;
    void* tim_handle;
    void* enc_handle;
};

};  // namespace motor_hardware
