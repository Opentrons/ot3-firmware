#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"

motor_handler::Empty empty{};
auto motor_handler::Empty::get_default() -> Empty& { return empty; }