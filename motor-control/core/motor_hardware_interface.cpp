#include "motor-control/core/motor_hardware_interface.hpp"

using namespace motor_hardware;

[[nodiscard]] auto StepperMotorHardwareIface::get_position_tracker() const
    -> q31_31 {
    return position_tracker.load();
}

auto StepperMotorHardwareIface::reset_position_tracker() -> void {
    set_position_tracker(0);
}

auto StepperMotorHardwareIface::set_position_tracker(q31_31 val) -> void {
    position_tracker.store(val);
}

auto StepperMotorHardwareIface::increment_position_tracker(q31_31 inc)
    -> std::pair<q31_31, q31_31> {
    std::pair<q31_31, q31_31> ret;
    ret.first = position_tracker.fetch_add(inc);
    ret.second = ret.first + inc;
    return ret;
}