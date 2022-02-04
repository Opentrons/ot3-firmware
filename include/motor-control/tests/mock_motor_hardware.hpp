#pragma once

#include "motor-control/core/motor_hardware_interface.hpp"

namespace test_mocks {

class MockMotorHardware : public motor_hardware::MotorHardwareIface {
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
    bool check_limit_switch() final { return false; }
    void set_LED(bool status) final {}
};

};  // namespace test_mocks
