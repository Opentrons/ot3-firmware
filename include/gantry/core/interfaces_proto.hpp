#pragma once

#include "can/core/can_bus.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "spi/core/spi.hpp"

namespace interfaces {

using DefinedMotorHardware = motor_hardware::MotorHardware;
/**
 * Initialize the hardware portability layer.
 */
void initialize();

/**
 * Get the CAN bus interface.
 * @return the can bus
 */
auto get_can_bus() -> can::bus::CanBus &;

/**
 * Get the SPI interface
 * @return the SPI interface
 */
auto get_spi() -> spi::hardware::SpiDeviceBase &;
auto get_motor_hardware_task()
    -> motor_hardware_task::MotorHardwareTask<DefinedMotorHardware> &;
/**
 * Get the motor hardware interface
 * @return the motor hardware interface
 */
auto get_motor_hardware_iface() -> DefinedMotorHardware &;

/**
 * Access to the global motor.
 *
 * @return The motor.
 */
auto get_motor() -> motor_class::Motor<lms::BeltConfig, DefinedMotorHardware> &;

/**
 * Access to the global motor driver configs.
 *
 * @return The motor driver configs.
 */
auto get_driver_config() -> tmc2130::configs::TMC2130DriverConfig &;

}  // namespace interfaces
