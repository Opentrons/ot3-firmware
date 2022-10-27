#pragma once

#include "motor-control/simulation/sim_motor_hardware_iface.hpp"

namespace z_motor_iface {

auto get_z_motor_interface()
    -> sim_motor_hardware_iface::SimMotorHardwareIface&;

}