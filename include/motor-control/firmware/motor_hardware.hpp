#pragma once

#include <cstdint>

#include "motor-control/core/motor_hardware_interface.hpp"

namespace motor_hardware {

struct PinConfig {
    void* port;
    uint16_t pin;
    uint8_t active_setting;
};

struct HardwareConfig {
    PinConfig direction;
    PinConfig step;
    PinConfig enable;
    PinConfig limit_switch;
    PinConfig led;
};

class MotorHardware : public MotorHardwareIface {
  public:
    ~MotorHardware() final = default;
    MotorHardware() = delete;
    MotorHardware(const HardwareConfig& config, void* timer_handle)
        : pins(config), tim_handle(timer_handle) {}
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
    bool check_limit_switch() final;
    void set_LED(bool status) final;

  private:
    HardwareConfig pins;
    void* tim_handle;
};

};  // namespace motor_hardware
