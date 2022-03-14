#pragma once

#include "can/core/can_bus.hpp"
#include "common/core/spi.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"

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
auto get_spi() -> spi::SpiDeviceBase &;

/**
 * Get the motor hardware interface
 * @return the motor hardware interface
 */
auto get_motor_hardware_iface() -> motor_hardware::StepperMotorHardwareIface &;

/**
 * Access to the global motor.
 *
 * @return The motor.
 */
auto get_motor() -> motor_class::Motor<lms::BeltConfig> &;

}  // namespace interfaces
