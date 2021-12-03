#pragma once

#include "can/core/can_bus.hpp"
#include "motor-control/core/spi.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"

namespace interfaces {

/**
 *
 * @return
 */
auto get_can_bus() -> can_bus::CanBus &;

/**
 *
 * @return
 */
auto get_spi() -> spi::TMC2130Spi &;

/**
 *
 * @return
 */
auto get_motor_hardware_iface() -> motor_hardware::MotorHardwareIface&;

}  // namespace interfaces