#pragma once

#include <atomic>
#include <cstdint>

#include "common/firmware/gpio.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/types.hpp"
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

class BrushedMotorHardware {
  public:
    ~BrushedMotorHardware() = default;
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
    void positive_direction();
    void negative_direction();
    void activate_motor();
    void deactivate_motor();
    auto check_limit_switch() -> bool { return limit; }
    auto check_estop_in() -> bool { return estop; }
    auto check_sync_in() -> bool { return sync; }
    void read_limit_switch();
    void read_estop_in();
    void read_sync_in();
    void grip();
    void ungrip();
    void stop_pwm();
    auto get_encoder_pulses() -> int32_t;
    void reset_encoder_pulses();
    void start_timer_interrupt();
    void stop_timer_interrupt();

    void encoder_overflow(int32_t direction);

    auto update_control(int32_t encoder_error) -> double;
    void reset_control();
    void set_stay_enabled(bool state) { stay_enabled = state; }
    auto get_stay_enabled() -> bool { return stay_enabled; }

    MotorPositionStatus position_flags{};

  private:
    bool stay_enabled = false;
    std::atomic_bool estop = false;
    std::atomic_bool limit = false;
    std::atomic_bool sync = false;
    BrushedHardwareConfig pins;
    void* enc_handle;
    int32_t motor_encoder_overflow_count = 0;
    ot_utils::pid::PID controller_loop;
    std::atomic<ControlDirection> control_dir = ControlDirection::unset;
};

};  // namespace motor_hardware
