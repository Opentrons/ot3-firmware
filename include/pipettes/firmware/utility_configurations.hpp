#pragma once

#include <optional>

#include "common/firmware/gpio.hpp"
#include "pipettes/core/pipette_type.h"
#include "sensors/firmware/sensor_hardware.hpp"

namespace utility_configs {

struct SensorHardwareGPIO {
    sensors::hardware::SensorHardwareConfiguration primary;
    std::optional<sensors::hardware::SensorHardwareConfiguration> secondary =
        std::nullopt;
};

struct SensorHardwareContainer {
    sensors::hardware::SensorHardware primary;
    std::optional<sensors::hardware::SensorHardware> secondary = std::nullopt;
};

auto led_gpio(PipetteType pipette_type) -> gpio::PinConfig;

auto get_sensor_hardware_container(SensorHardwareGPIO pins, sensors::hardware::SensorHardwareVersionSingleton& version_wrapper)
    -> SensorHardwareContainer;

template <PipetteType P>
auto sensor_configurations()
    -> std::enable_if_t<P == PipetteType::SINGLE_CHANNEL, SensorHardwareGPIO>;

template <PipetteType P>
auto sensor_configurations()
    -> std::enable_if_t<P == PipetteType::EIGHT_CHANNEL, SensorHardwareGPIO>;

template <PipetteType P>
auto sensor_configurations()
    -> std::enable_if_t<P == PipetteType::NINETY_SIX_CHANNEL,
                        SensorHardwareGPIO>;

template <PipetteType P>
auto sensor_configurations()
    -> std::enable_if_t<P == PipetteType::THREE_EIGHTY_FOUR_CHANNEL,
                        SensorHardwareGPIO>;

template <>
auto sensor_configurations<PipetteType::SINGLE_CHANNEL>() -> SensorHardwareGPIO;

template <>
auto sensor_configurations<PipetteType::EIGHT_CHANNEL>() -> SensorHardwareGPIO;

template <>
auto sensor_configurations<PipetteType::NINETY_SIX_CHANNEL>()
    -> SensorHardwareGPIO;

template <>
auto sensor_configurations<PipetteType::THREE_EIGHTY_FOUR_CHANNEL>()
    -> SensorHardwareGPIO;

}  // namespace utility_configs
