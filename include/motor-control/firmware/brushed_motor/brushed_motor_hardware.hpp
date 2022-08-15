#pragma once

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
};

// TODO tune the PID loop
constexpr double PID_KP = 1.35;
constexpr double PID_KI = 0.0005;
constexpr double PID_KD = 0.01;
//constexpr double PID_WL_H = 1;
//constexpr double PID_WL_L = 1;

class BrushedMotorHardware : public BrushedMotorHardwareIface {
  public:
    ~BrushedMotorHardware() final = default;
    BrushedMotorHardware() = delete;
    BrushedMotorHardware(const BrushedHardwareConfig& config,
                         void* encoder_handle, double encoder_timer_freq)
        : pins(config),
          enc_handle(encoder_handle),
          controller_loop{PID_KP,   PID_KI,  PID_KD, 1 / encoder_timer_freq} {}
                          //PID_WL_H, PID_WL_L} {}
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
};

};  // namespace motor_hardware
