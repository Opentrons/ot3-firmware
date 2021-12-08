#pragma once

#include "can/core/can_bus.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/spi.hpp"
#include "motor-control/core/motor.hpp"

namespace interfaces {

/**
 * Initialize the hardware portability layer.
 */
void initialize();

/**
 * Get the CAN bus interface.
 * @return the can bus
 */
auto get_can_bus() -> can_bus::CanBus &;

/**
 * Get the SPI interface
 * @return the SPI interface
 */
auto get_spi() -> spi::TMC2130Spi &;

/**
 * Get the motor hardware interface
 * @return the motor hardware interface
 */
auto get_motor_hardware_iface() -> motor_hardware::MotorHardwareIface &;


auto get_motor() -> motor_class::Motor<lms::BeltConfig> &;


}  // namespace interfaces
