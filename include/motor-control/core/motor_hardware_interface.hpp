#pragma once

#include <cstdint>

namespace motor_hardware {

class MotorHardwareIface {
  public:
    MotorHardwareIface() = default;
    MotorHardwareIface(const MotorHardwareIface&) = default;
    MotorHardwareIface(MotorHardwareIface&&) = default;
    auto operator=(MotorHardwareIface&&) -> MotorHardwareIface& = default;
    auto operator=(const MotorHardwareIface&) -> MotorHardwareIface& = default;
    virtual ~MotorHardwareIface() = default;
    virtual void positive_direction() = 0;
    virtual void negative_direction() = 0;
    virtual void activate_motor() = 0;
    virtual void deactivate_motor() = 0;
    virtual auto check_limit_switch() -> bool = 0;
    virtual auto check_sync_in() -> bool = 0;
    virtual auto get_encoder_pulses() -> int32_t = 0;
    virtual void reset_encoder_pulses() = 0;
    virtual void start_timer_interrupt() = 0;
    virtual void stop_timer_interrupt() = 0;
};

class StepperMotorHardwareIface : virtual public MotorHardwareIface {
  public:
    virtual void step() = 0;
    virtual void unstep() = 0;
    virtual void set_LED(bool status) = 0;
};

class PipetteStepperMotorHardwareIface
    : virtual public StepperMotorHardwareIface {
  public:
    virtual auto check_tip_sense() -> bool = 0;
};

class BrushedMotorHardwareIface : virtual public MotorHardwareIface {
  public:
    virtual void grip() = 0;
    virtual void ungrip() = 0;
    virtual void stop_pwm() = 0;
};
};  // namespace motor_hardware
