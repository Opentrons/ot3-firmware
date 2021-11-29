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
};

};  // namespace motor_hardware
