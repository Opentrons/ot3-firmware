#pragma once

#include <atomic>
#include <cstdint>
#include <utility>

#include "motor-control/core/types.hpp"

namespace motor_hardware {

class MotorHardwareIface {
  public:
    MotorHardwareIface() = default;
    MotorHardwareIface(const MotorHardwareIface&) = delete;
    MotorHardwareIface(MotorHardwareIface&&) = delete;
    auto operator=(MotorHardwareIface&&) -> MotorHardwareIface& = delete;
    auto operator=(const MotorHardwareIface&) -> MotorHardwareIface& = delete;
    virtual ~MotorHardwareIface() = default;
    virtual void positive_direction() = 0;
    virtual void negative_direction() = 0;
    virtual void activate_motor() = 0;
    virtual void deactivate_motor() = 0;
    virtual auto check_limit_switch() -> bool = 0;
    virtual auto check_estop_in() -> bool = 0;
    virtual auto check_sync_in() -> bool = 0;
    virtual void read_limit_switch() = 0;
    virtual void read_estop_in() = 0;
    virtual void read_sync_in() = 0;
    virtual auto get_encoder_pulses() -> int32_t = 0;
    virtual void reset_encoder_pulses() = 0;
    virtual void start_timer_interrupt() = 0;
    virtual void stop_timer_interrupt() = 0;

    // This variable can remain public because the only public methods
    // to it are thread-safe anyways.
    MotorPositionStatus position_flags{};
};

class StepperMotorHardwareIface : virtual public MotorHardwareIface {
  public:
    virtual void step() = 0;
    virtual void unstep() = 0;
    virtual void set_LED(bool status) = 0;

    // Position tracker interface is the same for all steppers

    /**
     * @brief Get the current position tracker atomically
     */
    [[nodiscard]] auto get_step_tracker() const -> uint32_t;

    /**
     * @brief Reset the position tracker to 0
     */
    auto reset_step_tracker() -> void;

    /**
     * @brief Set the position tracker to a specified value
     */
    auto set_step_tracker(uint32_t) -> void;

  private:
    // Used to track the position in microsteps.
    std::atomic<uint32_t> step_tracker{0};
};

class PipetteStepperMotorHardwareIface
    : virtual public StepperMotorHardwareIface {
  public:
    virtual auto check_tip_sense() -> bool = 0;
    virtual void read_tip_sense() = 0;
};

class BrushedMotorHardwareIface : virtual public MotorHardwareIface {
  public:
    virtual void grip() = 0;
    virtual void ungrip() = 0;
    virtual void stop_pwm() = 0;
    virtual auto update_control(int32_t encoder_error) -> double = 0;
    virtual void reset_control() = 0;
};
};  // namespace motor_hardware
