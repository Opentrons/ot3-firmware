#pragma once

#include <type_traits>

#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "pipettes/core/interfaces.hpp"
#include "pipettes/core/pipette_type.h"
#include "sensors/core/sensor_hardware_interface.hpp"

namespace motor_configs {
using namespace tmc2130::configs;
using namespace tmc2160::configs;

enum class TMC2130PipetteAxis {
    linear_motor,
};

enum class TMC2160PipetteAxis {
    left_gear_motor,
    right_gear_motor,
    linear_motor,
};

struct LowThroughputPipetteDriverHardware {
    TMC2130DriverConfig linear_motor;
};

struct HighThroughputPipetteDriverHardware {
    TMC2160DriverConfig right_gear_motor;
    TMC2160DriverConfig left_gear_motor;
    TMC2160DriverConfig linear_motor;
};

struct LowThroughputPipetteMotorHardware {
    motor_hardware::HardwareConfig linear_motor;
};

struct HighThroughputPipetteMotorHardware {
    motor_hardware::HardwareConfig right_gear_motor;
    motor_hardware::HardwareConfig left_gear_motor;
    motor_hardware::HardwareConfig linear_motor;
};

struct LowThroughputMotorConfigurations {
    LowThroughputPipetteMotorHardware hardware_pins{};
    LowThroughputPipetteDriverHardware driver_configs{};
};

struct HighThroughputMotorConfigurations {
    HighThroughputPipetteMotorHardware hardware_pins{};
    HighThroughputPipetteDriverHardware driver_configs{};
};

auto driver_config_by_axis(TMC2130PipetteAxis which)
    -> tmc2130::configs::TMC2130DriverConfig;

auto driver_config_by_axis(TMC2160PipetteAxis which)
    -> tmc2160::configs::TMC2160DriverConfig;

auto hardware_config_by_axis(TMC2130PipetteAxis which)
    -> motor_hardware::HardwareConfig;

auto hardware_config_by_axis(TMC2160PipetteAxis which)
    -> motor_hardware::HardwareConfig;

template <PipetteType P>
auto motor_configurations()
    -> std::enable_if_t<P == PipetteType::SINGLE_CHANNEL,
                        LowThroughputMotorConfigurations>;

template <PipetteType P>
auto motor_configurations()
    -> std::enable_if_t<P == PipetteType::EIGHT_CHANNEL,
                        LowThroughputMotorConfigurations>;

template <PipetteType P>
auto motor_configurations()
    -> std::enable_if_t<P == PipetteType::NINETY_SIX_CHANNEL,
                        HighThroughputMotorConfigurations>;

template <PipetteType P>
auto motor_configurations()
    -> std::enable_if_t<P == PipetteType::THREE_EIGHTY_FOUR_CHANNEL,
                        HighThroughputMotorConfigurations>;

template <>
auto motor_configurations<PipetteType::SINGLE_CHANNEL>()
    -> LowThroughputMotorConfigurations;

template <>
auto motor_configurations<PipetteType::EIGHT_CHANNEL>()
    -> LowThroughputMotorConfigurations;

template <>
auto motor_configurations<PipetteType::NINETY_SIX_CHANNEL>()
    -> HighThroughputMotorConfigurations;

template <>
auto motor_configurations<PipetteType::THREE_EIGHTY_FOUR_CHANNEL>()
    -> HighThroughputMotorConfigurations;

}  // namespace motor_configs
