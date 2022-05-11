#pragma once

#include "motor-control/core/motor_hardware_interface.hpp"

namespace test_mocks {

class MockMotorHardware : public motor_hardware::StepperMotorHardwareIface {
  public:
    ~MockMotorHardware() final = default;
    MockMotorHardware() = default;
    MockMotorHardware(const MockMotorHardware&) = default;
    auto operator=(const MockMotorHardware&) -> MockMotorHardware& = default;
    MockMotorHardware(MockMotorHardware&&) = default;
    auto operator=(MockMotorHardware&&) -> MockMotorHardware& = default;
    void step() final {}
    void unstep() final {}
    void positive_direction() final {}
    void negative_direction() final {}
    void activate_motor() final {}
    void deactivate_motor() final {}
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
    bool check_limit_switch() final { return mock_lim_sw_value; }
    bool check_sync_in() final { return mock_sync_value; }
    void set_LED(bool) final {}
    void set_mock_lim_sw(bool value) { mock_lim_sw_value = value; }
    void set_mock_sync_line(bool value) { mock_sync_value = value; }
    void set_finished_ack_id(uint8_t id) { finished_move_id = id; }
    uint8_t get_finished_ack_id() { return finished_move_id; }
    void reset_encoder_pulses() final { test_pulses = 0; }
    uint32_t get_encoder_pulses() final { return test_pulses; }
    void clear_encoder_SR() final {}
    auto get_encoder_SR_flag() -> bool final { return mock_sr_value; }
    auto get_encoder_direction() -> bool final { return mock_dir_value; }
    void sim_set_encoder_pulses(uint32_t pulses) { test_pulses = pulses; }

  private:
    bool mock_lim_sw_value = false;
    bool mock_sync_value = false;
    bool mock_sr_value = false;
    bool mock_dir_value = false;
    uint8_t finished_move_id = 0x0;
    uint32_t test_pulses = 0x0;
};

};  // namespace test_mocks
