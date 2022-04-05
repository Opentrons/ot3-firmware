#pragma once

#include "can/core/can_bus.hpp"
#include "common/core/spi.hpp"
#include "motor-control/core/brushed_motor/driver_interface.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/firmware/brushed_motor_hardware.hpp"

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
auto get_motor_hardware_iface() -> motor_hardware::MotorHardwareIface &;

/**
 * Access to the global motor.
 *
 * @return The motor.
 */
auto get_z_motor() -> motor_class::Motor<lms::LeadScrewConfig> &;

/**
 * Get the brushed motor hardware interface
 * @return the brushedmotor hardware interface
 */
auto get_brushed_motor_hardware_iface()
    -> motor_hardware::BrushedMotorHardwareIface &;

/**
 * Get the brushed motor driver hardware interface
 * @return the motor hardware interface
 */
auto get_brushed_motor_driver_hardware_iface()
    -> brushed_motor_driver::BrushedMotorDriverIface &;
}  // namespace interfaces
