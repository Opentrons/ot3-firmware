#include "pipettes/firmware/utility_configurations.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic pop

auto utility_configs::led_gpio(PipetteType pipette_type) -> gpio::PinConfig {
    switch (pipette_type) {
        case PipetteType::NINETY_SIX_CHANNEL:
        case PipetteType::THREE_EIGHTY_FOUR_CHANNEL:
            return gpio::PinConfig{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .port = GPIOC,
                .pin = GPIO_PIN_14,
                .active_setting = GPIO_PIN_RESET};
        case PipetteType::SINGLE_CHANNEL:
        case PipetteType::EIGHT_CHANNEL:
        default:
            return gpio::PinConfig{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .port = GPIOB,
                .pin = GPIO_PIN_11,
                .active_setting = GPIO_PIN_RESET};
    }
}

auto utility_configs::get_sensor_hardware_container(
    utility_configs::SensorHardwareGPIO pins,
    sensors::hardware::SensorHardwareVersionSingleton& version_wrapper)
    -> utility_configs::SensorHardwareContainer {
    if (pins.secondary.has_value()) {
        return utility_configs::SensorHardwareContainer{
            .primary = sensors::hardware::SensorHardware(pins.primary, version_wrapper),
            .secondary =
                sensors::hardware::SensorHardware(pins.secondary.value(), version_wrapper)};
    }
    return utility_configs::SensorHardwareContainer{
        .primary = sensors::hardware::SensorHardware(pins.primary, version_wrapper)};
}

template <>
auto utility_configs::sensor_configurations<PipetteType::SINGLE_CHANNEL>()
    -> utility_configs::SensorHardwareGPIO {
    auto pins = utility_configs::SensorHardwareGPIO{
        .primary = sensors::hardware::SensorHardwareConfiguration{
            .sync_out =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOB,
                 .pin = GPIO_PIN_6,
                 .active_setting = GPIO_PIN_RESET},
            .data_ready =
                gpio::PinConfig{
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOC,
                    .pin = GPIO_PIN_3,
                    .active_setting = GPIO_PIN_RESET},
            .tip_sense = gpio::PinConfig{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .port = GPIOC,
                .pin = GPIO_PIN_2,
                .active_setting = GPIO_PIN_RESET}}};
    return pins;
}

template <>
auto utility_configs::sensor_configurations<PipetteType::EIGHT_CHANNEL>()
    -> utility_configs::SensorHardwareGPIO {
    auto pins = utility_configs::SensorHardwareGPIO{
        .primary =
            sensors::hardware::SensorHardwareConfiguration{
                .sync_out =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_6,
                     .active_setting = GPIO_PIN_RESET},
                .data_ready =
                    gpio::PinConfig{
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                        .port = GPIOB,
                        .pin = GPIO_PIN_1,
                        .active_setting = GPIO_PIN_RESET},
                .tip_sense =
                    gpio::PinConfig{
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                        .port = GPIOC,
                        .pin = GPIO_PIN_2,
                        .active_setting = GPIO_PIN_RESET}},
        .secondary = sensors::hardware::SensorHardwareConfiguration{
            .sync_out =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOB,
                 .pin = GPIO_PIN_6,
                 .active_setting = GPIO_PIN_RESET},
            .data_ready = gpio::PinConfig{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .port = GPIOC,
                .pin = GPIO_PIN_3,
                .active_setting = GPIO_PIN_RESET}}};
    return pins;
}

template <>
auto utility_configs::sensor_configurations<PipetteType::NINETY_SIX_CHANNEL>()
    -> utility_configs::SensorHardwareGPIO {
    auto pins = utility_configs::SensorHardwareGPIO{
        .primary =
            sensors::hardware::SensorHardwareConfiguration{
                .sync_out =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_9,
                     .active_setting = GPIO_PIN_RESET},
                .data_ready =
                    gpio::PinConfig{
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                        .port = GPIOC,
                        .pin = GPIO_PIN_6,
                        .active_setting = GPIO_PIN_RESET},
                .tip_sense =
                    gpio::PinConfig{
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                        .port = GPIOC,
                        .pin = GPIO_PIN_7,
                        .active_setting = GPIO_PIN_SET}},
        .secondary = sensors::hardware::SensorHardwareConfiguration{
            .sync_out =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOA,
                 .pin = GPIO_PIN_9,
                 .active_setting = GPIO_PIN_RESET},
            .data_ready =
                gpio::PinConfig{
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOC,
                    .pin = GPIO_PIN_11,
                    .active_setting = GPIO_PIN_RESET},
            .tip_sense = gpio::PinConfig{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .port = GPIOC,
                .pin = GPIO_PIN_12,
                .active_setting = GPIO_PIN_SET}}};
    return pins;
}

template <>
auto utility_configs::sensor_configurations<
    PipetteType::THREE_EIGHTY_FOUR_CHANNEL>()
    -> utility_configs::SensorHardwareGPIO {
    auto pins = utility_configs::SensorHardwareGPIO{
        .primary =
            sensors::hardware::SensorHardwareConfiguration{
                .sync_out =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_9,
                     .active_setting = GPIO_PIN_RESET},
                .data_ready =
                    gpio::PinConfig{
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                        .port = GPIOC,
                        .pin = GPIO_PIN_11,
                        .active_setting = GPIO_PIN_RESET},
                .tip_sense =
                    gpio::PinConfig{
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                        .port = GPIOC,
                        .pin = GPIO_PIN_12,
                        .active_setting = GPIO_PIN_SET}},
        .secondary = sensors::hardware::SensorHardwareConfiguration{
            .sync_out =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOA,
                 .pin = GPIO_PIN_9,
                 .active_setting = GPIO_PIN_RESET},
            .data_ready =
                gpio::PinConfig{
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOC,
                    .pin = GPIO_PIN_6,
                    .active_setting = GPIO_PIN_RESET},
            .tip_sense = gpio::PinConfig{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .port = GPIOC,
                .pin = GPIO_PIN_7,
                .active_setting = GPIO_PIN_SET}}};
    return pins;
}
