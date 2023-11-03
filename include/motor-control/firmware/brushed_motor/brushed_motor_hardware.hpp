#pragma once

#include <atomic>
#include <cstdint>

#include "common/core/debounce.hpp"
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
                         void* encoder_handle,
                         const UsageEEpromConfig& eeprom_config,
                         void* stopwatch_handle)
        : pins(config),
          enc_handle(encoder_handle),
          controller_loop{config.pid_kp,  config.pid_ki,
                          config.pid_kd,  1.F / config.encoder_interrupt_freq,
                          config.wl_high, config.wl_low},
          eeprom_config{eeprom_config},
          stopwatch_handle{stopwatch_handle} {}
    BrushedMotorHardware(const BrushedMotorHardware&) = delete;
    auto operator=(const BrushedMotorHardware&)
        -> BrushedMotorHardware& = delete;
    BrushedMotorHardware(BrushedMotorHardware&&) = delete;
    auto operator=(BrushedMotorHardware&&) -> BrushedMotorHardware& = delete;
    void positive_direction() final;
    void negative_direction() final;
    void activate_motor() final;
    void deactivate_motor() final;
    auto check_limit_switch() -> bool final { return limit.debounce_state(); }
    auto check_estop_in() -> bool final { return estop.debounce_state(); }
    auto check_sync_in() -> bool final { return sync.debounce_state(); }
    void read_limit_switch() final;
    void read_estop_in() final;
    void read_sync_in() final;
    bool read_tmc_diag0() final;
    void grip() final;
    void ungrip() final;
    void stop_pwm() final;
    auto get_encoder_pulses() -> int32_t final;
    void reset_encoder_pulses() final;
    void start_timer_interrupt() final;
    void stop_timer_interrupt() final;
    auto is_timer_interrupt_running() -> bool final;
    auto get_stopwatch_pulses(bool clear) -> uint16_t final;

    void encoder_overflow(int32_t direction);

    auto update_control(int32_t encoder_error) -> double final;
    void reset_control() final;
    void set_stay_enabled(bool state) final { stay_enabled = state; }
    auto get_stay_enabled() -> bool final { return stay_enabled; }
    // need any std::optional usage?
    auto get_cancel_request() -> CancelRequest final {
        CancelRequest exchange_request = {};
        return cancel_request.exchange(exchange_request);
    }
    void set_cancel_request(can::ids::ErrorSeverity error_severity,
                            can::ids::ErrorCode error_code) final {
        CancelRequest update_request{
            .severity = static_cast<uint8_t>(error_severity),
            .code = static_cast<uint8_t>(error_code)};
        cancel_request.store(update_request);
    }
    void clear_cancel_request() final {
        CancelRequest clear_request = {};
        cancel_request.store(clear_request);
    }
    auto get_usage_eeprom_config() -> const UsageEEpromConfig& final {
        return eeprom_config;
    }
    void disable_encoder() final {}
    void enable_encoder() final {}

    void set_motor_state(BrushedMotorState state) final { motor_state = state; }
    auto get_motor_state() -> BrushedMotorState final { return motor_state; }

  private:
    bool stay_enabled = false;
    debouncer::Debouncer estop = debouncer::Debouncer{};
    debouncer::Debouncer limit = debouncer::Debouncer{};
    debouncer::Debouncer sync = debouncer::Debouncer{};
    BrushedHardwareConfig pins;
    void* enc_handle;
    int32_t motor_encoder_overflow_count = 0;
    ot_utils::pid::PID controller_loop;
    std::atomic<ControlDirection> control_dir = ControlDirection::unset;
    std::atomic<CancelRequest> cancel_request = {};
    const UsageEEpromConfig& eeprom_config;
    void* stopwatch_handle;
    BrushedMotorState motor_state = BrushedMotorState::UNHOMED;
};

};  // namespace motor_hardware
