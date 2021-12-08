#pragma once

#include "motor-control/core/motor.hpp"

namespace gantry_motor {

/**
 * Accessor for the motor class.
 *
 * @return
 */
auto get_motor() -> motor_class::Motor<lms::BeltConfig> &;

}  // namespace gantry_motor