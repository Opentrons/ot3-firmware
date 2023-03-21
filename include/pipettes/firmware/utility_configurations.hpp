#pragma once

#include <optional>

#include "common/firmware/gpio.hpp"
#include "pipettes/core/pipette_type.h"

#include "sensors/firmware/sensor_hardware.hpp"

namespace utility_configs {

struct HighThroughputSensorHardwareGPIO {
    sensors::hardware::SensorHardwareConfiguration primary;
    sensors::hardware::SensorHardwareConfiguration secondary;
};

struct LowThroughputSensorHardwareGPIO {
    sensors::hardware::SensorHardwareConfiguration primary;
};

struct SensorHardwareContainer {
    sensors::hardware::SensorHardware primary;
    std::optional<sensors::hardware::SensorHardware> secondary = std::nullopt;
};

auto led_gpio(PipetteType pipette_type) -> gpio::PinConfig;

auto get_sensor_hardware_container(LowThroughputSensorHardwareGPIO pins) -> SensorHardwareContainer;

auto get_sensor_hardware_container(HighThroughputSensorHardwareGPIO pins) -> SensorHardwareContainer;

template <PipetteType P>
auto sensor_configurations()
    -> std::enable_if_t<P == PipetteType::SINGLE_CHANNEL,
                        LowThroughputSensorHardwareGPIO>;

template <PipetteType P>
auto sensor_configurations()
    -> std::enable_if_t<P == PipetteType::EIGHT_CHANNEL,
                        HighThroughputSensorHardwareGPIO>;

template <PipetteType P>
auto sensor_configurations()
    -> std::enable_if_t<P == PipetteType::NINETY_SIX_CHANNEL,
                        HighThroughputSensorHardwareGPIO>;

template <PipetteType P>
auto sensor_configurations()
    -> std::enable_if_t<P == PipetteType::THREE_EIGHTY_FOUR_CHANNEL,
                        HighThroughputSensorHardwareGPIO>;

template <>
auto sensor_configurations<PipetteType::SINGLE_CHANNEL>()
    -> LowThroughputSensorHardwareGPIO;

template <>
auto sensor_configurations<PipetteType::EIGHT_CHANNEL>()
    -> HighThroughputSensorHardwareGPIO;

template <>
auto sensor_configurations<PipetteType::NINETY_SIX_CHANNEL>()
    -> HighThroughputSensorHardwareGPIO;

template <>
auto sensor_configurations<PipetteType::THREE_EIGHTY_FOUR_CHANNEL>()
    -> HighThroughputSensorHardwareGPIO;

}  // namespace utility_configs
