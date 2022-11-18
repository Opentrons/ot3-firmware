#pragma once

#include <atomic>
#include <cstdint>

#include "common/firmware/gpio.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "ot_utils/core/pid.hpp"

namespace motor_hardware {

struct PwmConfig {
    void* tim;
    uint32_t channel;
};

struct BrushedHardwareConfig {
    PwmConfig pwm_1;
    PwmConfig pwm_2;
    gpio::PinConfig enable;
    gpio::PinConfig limit_switch;
    gpio::PinConfig sync_in;
    gpio::PinConfig estop_in;
    double encoder_interrupt_freq;
    double pid_kp;
    double pid_ki;
    double pid_kd;
    double wl_high;
    double wl_low;
};

enum class ControlDirection { positive, negative, unset };

class BrushedMotorHardware : public BrushedMotorHardwareIface {
  public:
    ~BrushedMotorHardware() final = default;
    BrushedMotorHardware() = delete;
    BrushedMotorHardware(const BrushedHardwareConfig& config,
                         void* encoder_handle)
        : pins(config),
          enc_handle(encoder_handle),
          controller_loop{config.pid_kp,  config.pid_ki,
                          config.pid_kd,  1.F / config.encoder_interrupt_freq,
                          config.wl_high, config.wl_low} {}
    BrushedMotorHardware(const BrushedMotorHardware&) = delete;
    auto operator=(const BrushedMotorHardware&)
        -> BrushedMotorHardware& = delete;
    BrushedMotorHardware(BrushedMotorHardware&&) = delete;
    auto operator=(BrushedMotorHardware&&) -> BrushedMotorHardware& = delete;
    void positive_direction() final;
    void negative_direction() final;
    void activate_motor() final;
    void deactivate_motor() final;
    auto check_limit_switch() -> bool final;
    void grip() final;
    void ungrip() final;
    void stop_pwm() final;
    auto check_sync_in() -> bool final;
    auto get_encoder_pulses() -> int32_t final;
    void reset_encoder_pulses() final;
    void start_timer_interrupt() final;
    void stop_timer_interrupt() final;

    void encoder_overflow(int32_t direction);

    auto update_control(int32_t encoder_error) -> double final;
    void reset_control() final;

  private:
    BrushedHardwareConfig pins;
    void* enc_handle;
    int32_t motor_encoder_overflow_count = 0;
    ot_utils::pid::PID controller_loop;
    std::atomic<ControlDirection> control_dir = ControlDirection::unset;
};

};  // namespace motor_hardware
