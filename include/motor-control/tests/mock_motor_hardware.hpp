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
    bool check_sync_in() final { return true; }
    void set_LED(bool status) final {}
    void set_mock_lim_sw(bool value) { mock_lim_sw_value = value; }
    void set_finished_ack_id(uint8_t id) { finished_move_id = id; }
    uint8_t get_finished_ack_id() { return finished_move_id; }
    void reset_encoder_pulses() final { test_pulses = 0; }
    uint32_t get_encoder_pulses() { return test_pulses; }
    void sim_set_encoder_pulses(uint32_t pulses) { test_pulses = pulses; }

  private:
    bool mock_lim_sw_value = false;
    uint8_t finished_move_id = 0x0;
    uint32_t test_pulses = 0;
};

};  // namespace test_mocks
