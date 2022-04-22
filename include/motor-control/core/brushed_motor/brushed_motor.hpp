#pragma once

#include "driver_interface.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"

namespace brushed_motor {

struct BrushedMotor {
  public:
    BrushedMotor(motor_hardware::BrushedMotorHardwareIface& hardware_iface,
                 brushed_motor_driver::BrushedMotorDriverIface& driver_iface)
        : motion_controller(hardware_iface), driver(driver_iface) {}

    motor_hardware::BrushedMotorHardwareIface& motion_controller;
    brushed_motor_driver::BrushedMotorDriverIface& driver;

    BrushedMotor(const BrushedMotor&) = delete;
    auto operator=(const BrushedMotor&) -> BrushedMotor& = delete;
    BrushedMotor(BrushedMotor&&) = delete;
    auto operator=(BrushedMotor&&) -> BrushedMotor&& = delete;
    ~BrushedMotor() = default;
};

}  // namespace brushed_motor