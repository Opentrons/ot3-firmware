#pragma once

namespace brushed_motor_hardware {

class MotorHardwareIface {
  public:
    MotorHardwareIface() = default;
    MotorHardwareIface(const MotorHardwareIface&) = default;
    MotorHardwareIface(MotorHardwareIface&&) = default;
    auto operator=(MotorHardwareIface&&) -> MotorHardwareIface& = default;
    auto operator=(const MotorHardwareIface&) -> MotorHardwareIface& = default;
    virtual ~MotorHardwareIface() = default;
    virtual void activate_motor() = 0;
    virtual void deactivate_motor() = 0;
    virtual auto check_limit_switch() -> bool = 0;
};

};  // namespace brushed_motor_hardware
