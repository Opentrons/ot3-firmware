#pragma once

#include "motor-control/simulation/sim_motor_hardware_iface.hpp"

namespace z_motor_iface {

extern "C" using diag0_handler = void(*)();

auto get_z_motor_interface()
    -> sim_motor_hardware_iface::SimMotorHardwareIface&;

auto get_brushed_motor_interface()
    -> sim_motor_hardware_iface::SimBrushedMotorHardwareIface&;

}  // namespace z_motor_iface