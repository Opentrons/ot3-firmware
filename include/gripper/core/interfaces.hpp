#pragma once

#include "can/core/can_bus.hpp"
#include "common/core/spi.hpp"
#include "motor-control/core/brushed_motor/driver_interface.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/firmware/brushed_motor/brushed_motor_hardware.hpp"

namespace z_motor_iface {

void initialize();

/**
 * Get the motor hardware interface
 * @return the motor hardware interface
 */
auto get_motor_hardware_iface() -> motor_hardware::MotorHardwareIface &;

/**
 * Access to the global motor.
 *
 * @return The motor.
 */
auto get_z_motor() -> motor_class::Motor<lms::LeadScrewConfig> &;

}  // namespace z_motor_iface

namespace grip_motor_iface {

void initialize();

/**
 * Get the brushed motor hardware interface
 * @return the brushedmotor hardware interface
 */
auto get_motor_hardware_iface() -> motor_hardware::BrushedMotorHardwareIface &;

/**
 * Get the brushed motor driver hardware interface
 * @return the motor hardware interface
 */
auto get_motor_driver_hardware_iface()
    -> brushed_motor_driver::BrushedMotorDriverIface &;

}  // namespace grip_motor_iface
