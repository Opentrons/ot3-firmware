#include "motor-control/core/motor_hardware_interface.hpp"

using namespace motor_hardware;

[[nodiscard]] auto StepperMotorHardwareIface::get_step_tracker() const
    -> uint32_t {
    return step_tracker.load();
}

auto StepperMotorHardwareIface::reset_step_tracker() -> void {
    set_step_tracker(0);
}

auto StepperMotorHardwareIface::set_step_tracker(uint32_t val) -> void {
    step_tracker.store(val);
}
