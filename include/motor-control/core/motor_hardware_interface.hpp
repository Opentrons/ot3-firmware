#pragma once

namespace motor_hardware {

class MotorHardwareIface {
  public:
    MotorHardwareIface() = default;
    MotorHardwareIface(const MotorHardwareIface&) = default;
    MotorHardwareIface(MotorHardwareIface&&) = default;
    auto operator=(MotorHardwareIface&&) -> MotorHardwareIface& = default;
    auto operator=(const MotorHardwareIface&) -> MotorHardwareIface& = default;
    virtual ~MotorHardwareIface() = default;
    virtual void step() = 0;
    virtual void unstep() = 0;
    virtual void positive_direction() = 0;
    virtual void negative_direction() = 0;
    virtual void activate_motor() = 0;
    virtual void deactivate_motor() = 0;
    virtual void start_timer_interrupt() = 0;
    virtual void stop_timer_interrupt() = 0;
    virtual auto check_limit_switch() -> bool = 0;
    virtual void set_LED(bool status) = 0;
    virtual auto start_digital_analog_converter() -> bool = 0;
    virtual auto stop_digital_analog_converter() -> bool = 0;
    virtual auto set_reference_voltage(float val) -> bool = 0;
};

};  // namespace motor_hardware
