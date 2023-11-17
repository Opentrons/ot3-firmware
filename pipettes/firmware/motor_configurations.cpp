#include "pipettes/core/motor_configurations.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic pop

auto motor_configs::driver_config_by_axis(TMC2160PipetteAxis which)
    -> tmc2160::configs::TMC2160DriverConfig {
    switch (which) {
        case TMC2160PipetteAxis::left_gear_motor:
            return tmc2160::configs::TMC2160DriverConfig{
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
                                           .mres = 0x4},
                              .coolconf = {.sgt = 0x6},
                              .glob_scale = {.global_scaler = 0xa7}},
                .current_config =
                    {
                        .r_sense = 0.15,
                        .v_sf = 0.325,
                    },
                .chip_select = {
                    .cs_pin = GPIO_PIN_11,
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .GPIO_handle = GPIOB,
                }};
        case TMC2160PipetteAxis::right_gear_motor:
            return tmc2160::configs::TMC2160DriverConfig{
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
                                           .mres = 0x4},
                              .coolconf = {.sgt = 0x6},
                              .glob_scale = {.global_scaler = 0xa7}},
                .current_config =
                    {
                        .r_sense = 0.15,
                        .v_sf = 0.325,
                    },
                .chip_select = {
                    .cs_pin = GPIO_PIN_15,
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .GPIO_handle = GPIOC,
                }};
        case TMC2160PipetteAxis::linear_motor:
        default:
            return tmc2160::configs::TMC2160DriverConfig{
                .registers = {.gconfig = {.en_pwm_mode = 0},
                              .ihold_irun = {.hold_current = 16,
                                             .run_current = 31,
                                             .hold_current_delay = 0x7},
                              .tpowerdown = {},
                              .tcoolthrs = {.threshold = 0},
                              .thigh = {.threshold = 0x7},
                              .chopconf = {.toff = 0x2,
                                           .hstrt = 0x0,
                                           .hend = 0x3,
                                           .tbl = 0x0,
                                           .tpfd = 0x2,
                                           .mres = 0x4,
                                           .intpol = 0x1},
                              .coolconf = {.sgt = 0x6},
                              .glob_scale = {.global_scaler = 0xff}},
                .current_config =
                    {
                        .r_sense = 0.1,
                        .v_sf = 0.325,
                    },
                .chip_select = {
                    .cs_pin = GPIO_PIN_5,
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .GPIO_handle = GPIOC,
                }};
    }
}

auto motor_configs::driver_config_by_axis(TMC2130PipetteAxis which)
    -> tmc2130::configs::TMC2130DriverConfig {
    switch (which) {
        case TMC2130PipetteAxis::linear_motor:
        default:
            return tmc2130::configs::TMC2130DriverConfig{
                .registers = {.gconfig = {.en_pwm_mode = 0},
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
    -> motor_hardware::HardwareConfig {
    switch (which) {
        case TMC2130PipetteAxis::linear_motor:
        default:
            return motor_hardware::HardwareConfig{
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
                .sync_in =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_7,
                     .active_setting = GPIO_PIN_RESET},
                .estop_in =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_12,
                     .active_setting = GPIO_PIN_RESET},
            };
    }
}

auto motor_configs::hardware_config_by_axis(TMC2160PipetteAxis which)
    -> motor_hardware::HardwareConfig {
    switch (which) {
        case TMC2160PipetteAxis::right_gear_motor:
            return motor_hardware::HardwareConfig{
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
                .estop_in =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_9,
                     .active_setting = GPIO_PIN_RESET},
            };
        case TMC2160PipetteAxis::left_gear_motor:
            return motor_hardware::HardwareConfig{
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
                .estop_in =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_9,
                     .active_setting = GPIO_PIN_RESET},
            };
        case TMC2160PipetteAxis::linear_motor:
        default:
            return motor_hardware::HardwareConfig{
                .direction =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_6,
                     .active_setting = GPIO_PIN_RESET},
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
                .sync_in =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOD,
                     .pin = GPIO_PIN_2,
                     .active_setting = GPIO_PIN_RESET},
                .estop_in =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_9,
                     .active_setting = GPIO_PIN_RESET},
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
