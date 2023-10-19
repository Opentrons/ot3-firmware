#pragma once

#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"

namespace test_mocks {

class MockMotorHardware : public motor_hardware::StepperMotorHardwareIface {
  public:
    ~MockMotorHardware() final = default;
    MockMotorHardware() = default;
    MockMotorHardware(const MockMotorHardware&) = default;
    auto operator=(const MockMotorHardware&) -> MockMotorHardware& = default;
    MockMotorHardware(MockMotorHardware&&) = default;
    auto operator=(MockMotorHardware&&) -> MockMotorHardware& = default;
    void step() final { steps++; }
    void unstep() final {}
    void positive_direction() final {}
    void negative_direction() final {}
    void activate_motor() final {}
    void deactivate_motor() final {}
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
    bool is_timer_interrupt_running() final {
        return mock_timer_interrupt_running;
    }
    bool check_limit_switch() final { return mock_lim_sw_value; }
    bool check_estop_in() final { return mock_estop_in_value; }
    bool check_sync_in() final { return mock_sync_value; }
    void read_limit_switch() final {}
    void read_estop_in() final {}
    void read_sync_in() final {}
    void set_LED(bool) final {}
    void set_mock_lim_sw(bool value) { mock_lim_sw_value = value; }
    void set_mock_estop_in(bool value) { mock_estop_in_value = value; }
    void set_mock_sync_line(bool value) { mock_sync_value = value; }
    void set_finished_ack_id(uint8_t id) { finished_move_id = id; }
    uint8_t get_finished_ack_id() { return finished_move_id; }
    void reset_encoder_pulses() final { test_pulses = 0; }
    int32_t get_encoder_pulses() final { return test_pulses; }
    void sim_set_encoder_pulses(int32_t pulses) { test_pulses = pulses; }
    auto has_cancel_request() -> uint8_t final {
        uint8_t old_request = cancel_request;
        cancel_request = 0;
        return old_request;
    }
    void request_cancel(uint8_t error_severity) final { cancel_request = error_severity; }
    void sim_set_timer_interrupt_running(bool is_running) {
        mock_timer_interrupt_running = is_running;
    }
    auto steps_taken() -> uint64_t { return steps; }
    auto get_usage_eeprom_config() -> motor_hardware::UsageEEpromConfig& {
        return eeprom_config;
    }

    void disable_encoder() final {}
    void enable_encoder() final {}

  private:
    uint64_t steps = 0;
    bool mock_lim_sw_value = false;
    bool mock_estop_in_value = false;
    bool mock_sync_value = false;
    bool mock_sr_value = false;
    bool mock_dir_value = false;
    uint8_t finished_move_id = 0x0;
    int32_t test_pulses = 0x0;
    uint8_t cancel_request = 0;
    bool mock_timer_interrupt_running = true;
    motor_hardware::UsageEEpromConfig eeprom_config =
        motor_hardware::UsageEEpromConfig{
            std::array<UsageRequestSet, 1>{UsageRequestSet{
                .eeprom_key = 0,
                .type_key = uint16_t(
                    can::ids::MotorUsageValueType::linear_motor_distance),
                .length = usage_storage_task::distance_data_usage_len}}};
};

};  // namespace test_mocks
