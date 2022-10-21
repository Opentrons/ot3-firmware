#include "pipettes/core/motor_configurations.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic pop

auto motor_configs::driver_config_by_axis(TMC2160PipetteAxis which)
    -> tmc2160::configs::TMC2160DriverConfig {
    tmc2160::configs::TMC2160DriverConfig tmc2160_conf{
        .registers = {.gconfig = {.en_pwm_mode = 1},
                      .ihold_irun = {.hold_current = 16,
                                     .run_current = 31,
                                     .hold_current_delay = 0x7},
                      .tpowerdown = {},
                      .tcoolthrs = {.threshold = 0},
                      .thigh = {.threshold = 0xFFFFF},
                      .chopconf = {.toff = 0x5,
                                   .hstrt = 0x5,
                                   .hend = 0x3,
                                   .tbl = 0x2,
                                   .mres = 0x3},
                      .coolconf = {.sgt = 0x6},
                      .glob_scale = {.global_scaler = 0xA7}},
        .current_config =
            {
                .r_sense = 0.1,
                .v_sf = 0.325,
            },
        .chip_select = {
            .cs_pin = GPIO_PIN_6,
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .GPIO_handle = GPIOC,
        }};
    switch (which) {
        case TMC2160PipetteAxis::left_gear_motor:
            tmc2160_conf.chip_select = {
                .cs_pin = GPIO_PIN_9,
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .GPIO_handle = GPIOC,
            };
            return tmc2160_conf;
        case TMC2160PipetteAxis::right_gear_motor:
            tmc2160_conf.chip_select = {
                .cs_pin = GPIO_PIN_10,
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .GPIO_handle = GPIOC,
            };
            return tmc2160_conf;
        case TMC2160PipetteAxis::linear_motor:
        default:
            return tmc2160_conf;
    }
}

auto motor_configs::driver_config_by_axis(TMC2130PipetteAxis which)
    -> tmc2130::configs::TMC2130DriverConfig {
    switch (which) {
        case TMC2130PipetteAxis::linear_motor:
        default:
            return tmc2130::configs::TMC2130DriverConfig{
                .registers = {.gconfig = {.en_pwm_mode = 1},
                              .ihold_irun = {.hold_current = 0x2,
                                             .run_current = 0x10,
                                             .hold_current_delay = 0x7},
                              .tpowerdown = {},
                              .tcoolthrs = {.threshold = 0},
                              .thigh = {.threshold = 0xFFFFF},
                              .chopconf = {.toff = 0x5,
                                           .hstrt = 0x5,
                                           .hend = 0x3,
                                           .tbl = 0x2,
                                           .mres = 0x3},
                              .coolconf = {.sgt = 0x6}},
                .current_config =
                    {
                        .r_sense = 0.1,
                        .v_sf = 0.325,
                    },
                .chip_select = {
                    .cs_pin = GPIO_PIN_12,
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .GPIO_handle = GPIOB,
                }};
    }
}

auto motor_configs::hardware_config_by_axis(TMC2130PipetteAxis which)
    -> pipette_motor_hardware::HardwareConfig {
    switch (which) {
        case TMC2130PipetteAxis::linear_motor:
        default:
            return pipette_motor_hardware::HardwareConfig{
                .direction =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_6,
                     .active_setting = GPIO_PIN_RESET},
                .step =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_7,
                     .active_setting = GPIO_PIN_SET},
                .enable =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_10,
                     .active_setting = GPIO_PIN_SET},
                .limit_switch =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_6,
                     .active_setting = GPIO_PIN_SET},
                // LED PIN B11, active setting low
                .led = {},
                .tip_sense =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_2,
                     .active_setting = GPIO_PIN_SET},
            };
    }
}

auto motor_configs::hardware_config_by_axis(TMC2160PipetteAxis which)
    -> pipette_motor_hardware::HardwareConfig {
    switch (which) {
        case TMC2160PipetteAxis::right_gear_motor:
            return pipette_motor_hardware::HardwareConfig{
                .direction =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_7,
                     .active_setting = GPIO_PIN_SET},
                .step =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_2,
                     .active_setting = GPIO_PIN_SET},
                .enable =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_10,
                     .active_setting = GPIO_PIN_SET},
                .limit_switch =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_10,
                     .active_setting = GPIO_PIN_SET},
                // LED PIN C11, active setting low
                .led = {},
                .tip_sense =
                    {// Located on the back sensor board
                     // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_7,
                     .active_setting = GPIO_PIN_SET},
            };
        case TMC2160PipetteAxis::left_gear_motor:
            return pipette_motor_hardware::HardwareConfig{
                .direction =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_0,
                     .active_setting = GPIO_PIN_SET},
                .step =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_1,
                     .active_setting = GPIO_PIN_SET},
                .enable =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_10,
                     .active_setting = GPIO_PIN_SET},
                .limit_switch =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_12,
                     .active_setting = GPIO_PIN_SET},
                // LED PIN C11, active setting low
                .led = {},
                .tip_sense =
                    {// Located on the front sensor board
                     // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_12,
                     .active_setting = GPIO_PIN_SET},
            };
        case TMC2160PipetteAxis::linear_motor:
        default:
            return pipette_motor_hardware::HardwareConfig{
                .direction =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_6,
                     .active_setting = GPIO_PIN_SET},
                .step =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_7,
                     .active_setting = GPIO_PIN_SET},
                .enable =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_4,
                     .active_setting = GPIO_PIN_SET},
                .limit_switch =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_4,
                     .active_setting = GPIO_PIN_SET},
                // LED PIN C11, active setting low
                .led = {},
                // tip sense will be checked on the two pipette
                // tip pick up motors.
                // TODO need to think further about force pick up of tips
                .tip_sense = {},
            };
    }
}

template <>
auto motor_configs::motor_configurations<PipetteType::SINGLE_CHANNEL>()
    -> motor_configs::LowThroughputMotorConfigurations {
    auto configs = motor_configs::LowThroughputPipetteDriverHardware{
        .linear_motor =
            driver_config_by_axis(TMC2130PipetteAxis::linear_motor)};
    auto pins = motor_configs::LowThroughputPipetteMotorHardware{
        .linear_motor =
            hardware_config_by_axis(TMC2130PipetteAxis::linear_motor)};
    return motor_configs::LowThroughputMotorConfigurations{
        .hardware_pins = pins, .driver_configs = configs};
}

template <>
auto motor_configs::motor_configurations<PipetteType::EIGHT_CHANNEL>()
    -> motor_configs::LowThroughputMotorConfigurations {
    auto configs = motor_configs::LowThroughputPipetteDriverHardware{
        .linear_motor =
            driver_config_by_axis(TMC2130PipetteAxis::linear_motor)};
    auto pins = motor_configs::LowThroughputPipetteMotorHardware{
        .linear_motor =
            hardware_config_by_axis(TMC2130PipetteAxis::linear_motor)};
    return motor_configs::LowThroughputMotorConfigurations{
        .hardware_pins = pins, .driver_configs = configs};
}

template <>
auto motor_configs::motor_configurations<PipetteType::NINETY_SIX_CHANNEL>()
    -> motor_configs::HighThroughputMotorConfigurations {
    auto configs = motor_configs::HighThroughputPipetteDriverHardware{
        .right_gear_motor =
            driver_config_by_axis(TMC2160PipetteAxis::right_gear_motor),
        .left_gear_motor =
            driver_config_by_axis(TMC2160PipetteAxis::left_gear_motor),
        .linear_motor =
            driver_config_by_axis(TMC2160PipetteAxis::linear_motor)};
    auto pins = motor_configs::HighThroughputPipetteMotorHardware{
        .right_gear_motor =
            hardware_config_by_axis(TMC2160PipetteAxis::right_gear_motor),
        .left_gear_motor =
            hardware_config_by_axis(TMC2160PipetteAxis::left_gear_motor),
        .linear_motor =
            hardware_config_by_axis(TMC2160PipetteAxis::linear_motor),
    };
    return motor_configs::HighThroughputMotorConfigurations{
        .hardware_pins = pins, .driver_configs = configs};
}

template <>
auto motor_configs::motor_configurations<
    PipetteType::THREE_EIGHTY_FOUR_CHANNEL>()
    -> motor_configs::HighThroughputMotorConfigurations {
    auto configs = motor_configs::HighThroughputPipetteDriverHardware{
        .right_gear_motor =
            driver_config_by_axis(TMC2160PipetteAxis::right_gear_motor),
        .left_gear_motor =
            driver_config_by_axis(TMC2160PipetteAxis::left_gear_motor),
        .linear_motor =
            driver_config_by_axis(TMC2160PipetteAxis::linear_motor)};
    auto pins = motor_configs::HighThroughputPipetteMotorHardware{
        .right_gear_motor =
            hardware_config_by_axis(TMC2160PipetteAxis::right_gear_motor),
        .left_gear_motor =
            hardware_config_by_axis(TMC2160PipetteAxis::left_gear_motor),
        .linear_motor =
            hardware_config_by_axis(TMC2160PipetteAxis::linear_motor),
    };
    return motor_configs::HighThroughputMotorConfigurations{
        .hardware_pins = pins, .driver_configs = configs};
}

template <>
auto motor_configs::sensor_configurations<PipetteType::SINGLE_CHANNEL>()
    -> motor_configs::LowThroughputSensorHardware {
    auto pins = motor_configs::LowThroughputSensorHardware{
        .primary = sensors::hardware::SensorHardwareConfiguration{
            .sync_in =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOB,
                 .pin = GPIO_PIN_7,
                 .active_setting = GPIO_PIN_RESET},
            .sync_out =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOB,
                 .pin = GPIO_PIN_6,
                 .active_setting = GPIO_PIN_RESET},
            .data_ready =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOC,
                 .pin = GPIO_PIN_3,
                 .active_setting = GPIO_PIN_RESET},
        }};
    return pins;
}

// TODO (06/02/22 CM): change up the hardware grouping to accommodate both
//  pressure sensors on the EIGHT CHANNEL pipette
template <>
auto motor_configs::sensor_configurations<PipetteType::EIGHT_CHANNEL>()
    -> motor_configs::LowThroughputSensorHardware {
    auto pins = motor_configs::LowThroughputSensorHardware{
        .primary = sensors::hardware::SensorHardwareConfiguration{
            .sync_in =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOB,
                 .pin = GPIO_PIN_7,
                 .active_setting = GPIO_PIN_RESET},
            .sync_out =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOB,
                 .pin = GPIO_PIN_6,
                 .active_setting = GPIO_PIN_RESET},
            .data_ready =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOC,
                 .pin = GPIO_PIN_3,
                 .active_setting = GPIO_PIN_RESET},
        }};
    return pins;
}

template <>
auto motor_configs::sensor_configurations<PipetteType::NINETY_SIX_CHANNEL>()
    -> motor_configs::HighThroughputSensorHardware {
    auto pins = motor_configs::HighThroughputSensorHardware{
        .primary =
            sensors::hardware::SensorHardwareConfiguration{
                .sync_in =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOD,
                     .pin = GPIO_PIN_2,
                     .active_setting = GPIO_PIN_RESET},
                .sync_out =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_9,
                     .active_setting = GPIO_PIN_RESET},
                .data_ready =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_11,
                     .active_setting = GPIO_PIN_RESET},
            },
        .secondary = sensors::hardware::SensorHardwareConfiguration{
            .sync_in =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOD,
                 .pin = GPIO_PIN_2,
                 .active_setting = GPIO_PIN_RESET},
            .sync_out =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOA,
                 .pin = GPIO_PIN_9,
                 .active_setting = GPIO_PIN_RESET},
            .data_ready =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOC,
                 .pin = GPIO_PIN_6,
                 .active_setting = GPIO_PIN_RESET},
        }};
    return pins;
}

template <>
auto motor_configs::sensor_configurations<
    PipetteType::THREE_EIGHTY_FOUR_CHANNEL>()
    -> motor_configs::HighThroughputSensorHardware {
    auto pins = motor_configs::HighThroughputSensorHardware{
        .primary =
            sensors::hardware::SensorHardwareConfiguration{
                .sync_in =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOD,
                     .pin = GPIO_PIN_2,
                     .active_setting = GPIO_PIN_RESET},
                .sync_out =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_9,
                     .active_setting = GPIO_PIN_RESET},
                .data_ready =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_11,
                     .active_setting = GPIO_PIN_RESET},
            },
        .secondary = sensors::hardware::SensorHardwareConfiguration{
            .sync_in =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOD,
                 .pin = GPIO_PIN_2,
                 .active_setting = GPIO_PIN_RESET},
            .sync_out =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOA,
                 .pin = GPIO_PIN_9,
                 .active_setting = GPIO_PIN_RESET},
            .data_ready =
                {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                 .port = GPIOC,
                 .pin = GPIO_PIN_6,
                 .active_setting = GPIO_PIN_RESET},
        }};
    return pins;
}
