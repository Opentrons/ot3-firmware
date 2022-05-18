#pragma once

#include <type_traits>

#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/firmware/stepper_motor/pipette_motor_hardware.hpp"
#include "pipettes/core/pipette_type.h"

namespace interfaces {

using namespace tmc2130::configs;
using namespace tmc2160::configs;

enum class TMC2130PipetteAxis {
    left_gear_motor,
    right_gear_motor,
    linear_motor,
};

enum class TMC2160PipetteAxis {
    linear_motor,
};

struct LowThroughputPipetteDriverHardware {
    TMC2130DriverConfig linear_motor;
};

struct HighThroughputPipetteDriverHardware {
    TMC2130DriverConfig right_gear_motor;
    TMC2130DriverConfig left_gear_motor;
    TMC2160DriverConfig linear_motor;
};

struct LowThroughputPipetteMotorHardware {
    pipette_motor_hardware::HardwareConfig linear_motor;
};

struct HighThroughputPipetteMotorHardware {
    pipette_motor_hardware::HardwareConfig right_gear_motor;
    pipette_motor_hardware::HardwareConfig left_gear_motor;
    pipette_motor_hardware::HardwareConfig linear_motor;
};

auto driver_config_by_axis(TMC2130PipetteAxis which)
    -> tmc2130::configs::TMC2130DriverConfig;

auto driver_config_by_axis(TMC2160PipetteAxis which)
    -> tmc2160::configs::TMC2160DriverConfig;

auto hardware_config_by_axis(TMC2130PipetteAxis which)
    -> pipette_motor_hardware::HardwareConfig;

auto hardware_config_by_axis(TMC2160PipetteAxis which)
    -> pipette_motor_hardware::HardwareConfig;

template <PipetteType P>
auto hardware_config() -> std::enable_if_t<P == PipetteType::SINGLE_CHANNEL,
                                           LowThroughputPipetteMotorHardware>;

template <PipetteType P>
auto hardware_config() -> std::enable_if_t<P == PipetteType::NINETY_SIX_CHANNEL,
                                           HighThroughputPipetteMotorHardware>;

template <>
auto hardware_config<PipetteType::SINGLE_CHANNEL>()
    -> LowThroughputPipetteMotorHardware;

template <>
auto hardware_config<PipetteType::NINETY_SIX_CHANNEL>()
    -> HighThroughputPipetteMotorHardware;

template <PipetteType P>
auto driver_config() -> std::enable_if_t<P == PipetteType::SINGLE_CHANNEL,
                                         LowThroughputPipetteDriverHardware>;

template <PipetteType P>
auto driver_config() -> std::enable_if_t<P == PipetteType::NINETY_SIX_CHANNEL,
                                         HighThroughputPipetteDriverHardware>;

template <>
auto driver_config<PipetteType::SINGLE_CHANNEL>()
    -> LowThroughputPipetteDriverHardware;

template <>
auto driver_config<PipetteType::NINETY_SIX_CHANNEL>()
    -> HighThroughputPipetteDriverHardware;

}  // namespace interfaces