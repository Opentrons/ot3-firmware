#pragma once

#include "motor-control/core/brushed_motor/brushed_motor.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"

namespace z_motor_iface {

void initialize();

/**
 * Access to the z motor.
 *
 * @return The motor.
 */
auto get_z_motor() -> motor_class::Motor<lms::LeadScrewConfig> &;

}  // namespace z_motor_iface

namespace grip_motor_iface {

void initialize();

/**
 * Access to the grip motor.
 *
 * @return The motor.
 */
auto get_grip_motor() -> brushed_motor::BrushedMotor &;

}  // namespace grip_motor_iface
