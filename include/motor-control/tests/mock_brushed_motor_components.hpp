#pragma once

#include "motor-control/core/brushed_motor/driver_interface.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

namespace test_mocks {

using namespace brushed_motor_driver;
using namespace motor_hardware;

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
    void positive_direction() final {}
    void negative_direction() final {}
    void activate_motor() final {}
    void deactivate_motor() final {}
    auto check_limit_switch() -> bool final { return ls_val; }
    void grip() final { is_gripping = true; }
    void ungrip() final { is_gripping = false; }
    void stop_pwm() final {}
    auto check_sync_in() -> bool final { return sync_val; }
    auto get_encoder_pulses() -> int32_t final {
        return (motor_encoder_overflow_count << 16) + enc_val;
    }
    void reset_encoder_pulses() final { enc_val = 0; }
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
    void encoder_overflow(int32_t direction) {
        motor_encoder_overflow_count += direction;
    }
    auto get_limit_switch_val() -> bool { return ls_val; }
    auto get_is_gripping() -> bool { return is_gripping; }
    void set_encoder_value(int32_t val) { enc_val = val; }
    auto set_limit_switch(bool val) { ls_val = val; }

  private:
    int32_t motor_encoder_overflow_count = 0;
    bool ls_val = false;
    bool sync_val = false;
    bool is_gripping = false;
    int32_t enc_val = 0;
};

class MockBrushedMotorDriverIface : public BrushedMotorDriverIface {
  public:
    bool start_digital_analog_converter() final { return true; }
    bool stop_digital_analog_converter() final { return true; }
    bool set_reference_voltage(float) final { return true; }
    void setup() final {}
    void update_pwm_settings(uint32_t) final {}
};

struct MockBrushedMoveStatusReporterClient {
    void send_brushed_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m) {
        messages.push_back(m);
    }

    std::vector<move_status_reporter_task::TaskMessage> messages{};
};

};  // namespace test_mocks