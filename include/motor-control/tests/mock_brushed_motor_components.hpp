#pragma once
#include "motor-control/core/brushed_motor/driver_interface.hpp"
#include "motor-control/core/brushed_motor/error_tolerance_config.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "ot_utils/core/pid.hpp"

namespace test_mocks {

using namespace brushed_motor_driver;
using namespace motor_hardware;
using namespace error_tolerance_config;

enum class PWM_DIRECTION { positive, negative, unset };

class MockBrushedMotorHardware : public BrushedMotorHardwareIface {
  public:
    ~MockBrushedMotorHardware() final = default;
    MockBrushedMotorHardware() = default;
    MockBrushedMotorHardware(const MockBrushedMotorHardware&) = default;
    auto operator=(const MockBrushedMotorHardware&)
        -> MockBrushedMotorHardware& = default;
    MockBrushedMotorHardware(MockBrushedMotorHardware&&) = default;
    auto operator=(MockBrushedMotorHardware&&)
        -> MockBrushedMotorHardware& = default;
    void positive_direction() final { move_dir = PWM_DIRECTION::positive; }
    void negative_direction() final { move_dir = PWM_DIRECTION::negative; }
    void activate_motor() final {}
    void deactivate_motor() final {}
    auto check_limit_switch() -> bool final { return ls_val; }
    auto check_estop_in() -> bool final { return estop_in_val; }
    void read_limit_switch() final {}
    void read_estop_in() final {}
    void grip() final {
        positive_direction();
        is_gripping = true;
    }
    void ungrip() final {
        negative_direction();
        is_gripping = false;
    }
    void stop_pwm() final { move_dir = PWM_DIRECTION::unset; }
    auto check_sync_in() -> bool final { return sync_val; }
    void read_sync_in() final {}

    auto get_encoder_pulses() -> int32_t final {
        return (motor_encoder_overflow_count << 16) + enc_val;
    }
    void reset_encoder_pulses() final { enc_val = 0; }
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
    bool is_timer_interrupt_running() final { return timer_interrupt_running; }
    void encoder_overflow(int32_t direction) {
        motor_encoder_overflow_count += direction;
    }
    auto get_limit_switch_val() -> bool { return ls_val; }
    auto get_is_gripping() -> bool { return is_gripping; }
    void set_encoder_value(int32_t val) { enc_val = val; }
    auto set_limit_switch(bool val) { ls_val = val; }
    auto set_estop_in(bool val) { estop_in_val = val; }

    double update_control(int32_t encoder_error) {
        pid_controller_output = controller_loop.compute(encoder_error);
        return pid_controller_output;
    }
    void reset_control() { controller_loop.reset(); }
    double get_pid_controller_output() { return pid_controller_output; }
    PWM_DIRECTION get_direction() { return move_dir; }
    void set_stay_enabled(bool state) { stay_enabled = state; }
    auto get_stay_enabled() -> bool { return stay_enabled; }
    auto has_cancel_request() -> bool final {
        bool old_request = cancel_request;
        cancel_request = false;
        return old_request;
    }
    void request_cancel() final { cancel_request = true; }
    void set_timer_interrupt_running(bool is_running) {
        timer_interrupt_running = is_running;
    }
    auto get_usage_eeprom_config() -> motor_hardware::UsageEEpromConfig& {
        return eeprom_config;
    }

  private:
    bool stay_enabled = false;
    PWM_DIRECTION move_dir = PWM_DIRECTION::unset;
    int32_t motor_encoder_overflow_count = 0;
    bool ls_val = false;
    bool sync_val = false;
    bool estop_in_val = false;
    bool is_gripping = false;
    bool motor_enabled = false;
    double pid_controller_output = 0.0;
    int32_t enc_val = 0;
    // these controller loop values were selected just because testing
    // does not emulate change in speed and these give us pretty good values
    // when the "motor" instantly goes to top speed then instantly stops
    ot_utils::pid::PID controller_loop{0.008,         0.0045, 0.000015,
                                       1.F / 32000.0, 7,      -7};
    bool cancel_request = false;
    bool timer_interrupt_running = true;
    motor_hardware::UsageEEpromConfig eeprom_config =
        motor_hardware::UsageEEpromConfig{
            .usage_requests = std::vector<UsageRequestSet>{UsageRequestSet{
                .eeprom_key = 0,
                .type_key = uint16_t(
                    can::ids::MotorUsageValueType::linear_motor_distance),
                .length = usage_storage_task::distance_data_usage_len}}};
};

class MockBrushedMotorDriverIface : public BrushedMotorDriverIface {
  public:
    bool start_digital_analog_converter() final { return true; }
    bool stop_digital_analog_converter() final { return true; }
    bool set_reference_voltage(float v) final {
        v_ref = v;
        return true;
    }
    void setup() final {}
    void update_pwm_settings(uint32_t pwm_set) { pwm_val = pwm_set; }
    uint32_t get_pwm_settings() { return pwm_val; }
    uint32_t pwm_active_duty_clamp(uint32_t duty_cycle) {
        return std::clamp(duty_cycle, uint32_t(7), uint32_t(100));
    }
    [[nodiscard]] auto get_current_vref() const -> float final { return v_ref; }
    [[nodiscard]] auto get_current_duty_cycle() const -> uint32_t final {
        return pwm_val;
    }

  private:
    float v_ref = 0;
    uint32_t pwm_val = 0;
};

struct MockBrushedMoveStatusReporterClient {
    void send_brushed_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m) {
        messages.push_back(m);
    }

    std::vector<move_status_reporter_task::TaskMessage> messages{};
};

struct MockBrushedMotionController {
    void set_error_tolerance(
        const can::messages::SetGripperErrorToleranceRequest& can_msg) {
        error_config.update_tolerance(
            fixed_point_to_float(can_msg.max_pos_error_mm, S15Q16_RADIX),
            fixed_point_to_float(can_msg.max_unwanted_movement_mm,
                                 S15Q16_RADIX));
    }
    BrushedMotorErrorTolerance& error_config;
};

};  // namespace test_mocks
