#pragma once

#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "pipettes/core/pipette_type.h"

namespace interfaces {

using namespace tmc2130::configs;
using namespace tmc2160::configs;

enum class PipetteAxisType {
    left_gear_motor,
    right_gear_motor,
    linear_motor_high_throughput,
    linear_motor_low_throughput,
};

struct PipetteDriverHardware {
    TMC2130DriverConfig right_gear_motor;
    TMC2130DriverConfig left_gear_motor;
    TMC2130DriverConfig low_throughput_motor;
    TMC2160DriverConfig high_throughput_motor;
};

struct PipetteMotorHardware {
    motor_hardware::HardwareConfig right_gear_motor;
    motor_hardware::HardwareConfig left_gear_motor;
    motor_hardware::HardwareConfig low_throughput_motor;
    motor_hardware::HardwareConfig high_throughput_motor;
};

auto tmc2130_driver_config_by_axis(PipetteAxisType which)
    -> tmc2130::configs::TMC2130DriverConfig;

auto tmc2160_driver_config_by_axis() -> tmc2160::configs::TMC2160DriverConfig;

auto hardware_config_by_axis(PipetteAxisType which)
    -> motor_hardware::HardwareConfig;

auto hardware_config(PipetteType pipette) -> PipetteMotorHardware;

auto driver_config(PipetteType pipette) -> PipetteDriverHardware;
}  // namespace interfaces