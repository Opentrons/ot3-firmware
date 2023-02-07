#pragma once

#include "can/core/can_bus.hpp"
#include "motor-control/core/brushed_motor/brushed_motor.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "motor-control/firmware/brushed_motor/brushed_motor_hardware.hpp"
#include "spi/core/spi.hpp"

namespace z_motor_iface {

using DefinedZMotorHardware = motor_hardware::MotorHardware;

void initialize();

/**
 * Access to the z motor.
 *
 * @return The motor.
 */
auto get_z_motor()
    -> motor_class::Motor<lms::LeadScrewConfig, DefinedZMotorHardware> &;

/**
 * Get the SPI interface
 * @return the SPI interface
 */
auto get_spi() -> spi::hardware::SpiDeviceBase &;

/**
 * Get the tmc2130 motor driver configs
 * @return the motor driver configs
 */
auto get_tmc2130_driver_configs() -> tmc2130::configs::TMC2130DriverConfig &;

auto get_z_motor_hardware_task()
    -> motor_hardware_task::MotorHardwareTask<DefinedZMotorHardware> &;
}  // namespace z_motor_iface

namespace grip_motor_iface {

void initialize();

/**
 * Access to the grip motor.
 *
 * @return The motor.
 */
auto get_grip_motor()
    -> brushed_motor::BrushedMotor<lms::GearBoxConfig,
                                   motor_hardware::BrushedMotorHardware> &;

auto get_grip_motor_hardware_task() -> motor_hardware_task::MotorHardwareTask<
    motor_hardware::BrushedMotorHardware> &;

}  // namespace grip_motor_iface
