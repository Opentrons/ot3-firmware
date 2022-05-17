#include "pipettes/core/interfaces.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic pop

auto interfaces::tmc2160_driver_config_by_axis()
    -> tmc2160::configs::TMC2160DriverConfig {
    return tmc2160::configs::TMC2160DriverConfig{
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
                      .coolconf = {.sgt = 0x6},
                      .glob_scale = {.global_scaler = 0x70}},
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
}

auto interfaces::tmc2130_driver_config_by_axis(PipetteAxisType which)
    -> tmc2130::configs::TMC2130DriverConfig {
    tmc2130::configs::TMC2130DriverConfig tmc2130_conf{
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
            .cs_pin = GPIO_PIN_9,
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .GPIO_handle = GPIOC,
        }};
    switch (which) {
        case PipetteAxisType::left_gear_motor:
            return tmc2130_conf;
        case PipetteAxisType::right_gear_motor:
            tmc2130_conf.chip_select = {
                .cs_pin = GPIO_PIN_10,
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .GPIO_handle = GPIOC,
            };
            return tmc2130_conf;
        case PipetteAxisType::linear_motor_low_throughput:
        default:
            tmc2130_conf.chip_select = {
                .cs_pin = GPIO_PIN_6,
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                .GPIO_handle = GPIOC,
            };
            return tmc2130_conf;
    }
}

auto interfaces::hardware_config_by_axis(PipetteAxisType which)
    -> motor_hardware::HardwareConfig {
    switch (which) {
        case PipetteAxisType::right_gear_motor:
            return motor_hardware::HardwareConfig{
                .direction =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_13,
                     .active_setting = GPIO_PIN_SET},
                .step =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_8,
                     .active_setting = GPIO_PIN_SET},
                .enable =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOD,
                     .pin = GPIO_PIN_2,
                     .active_setting = GPIO_PIN_SET},
                .limit_switch =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_14,
                     .active_setting = GPIO_PIN_SET},
                // LED PIN C11, active setting low
                .led = {},
            };
        case PipetteAxisType::left_gear_motor:
            return motor_hardware::HardwareConfig{
                .direction =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_7,
                     .active_setting = GPIO_PIN_SET},
                .step =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_8,
                     .active_setting = GPIO_PIN_SET},
                .enable =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOD,
                     .pin = GPIO_PIN_2,
                     .active_setting = GPIO_PIN_SET},
                .limit_switch =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_10,
                     .active_setting = GPIO_PIN_SET},
                // LED PIN C11, active setting low
                .led = {},
            };
        case PipetteAxisType::linear_motor_high_throughput:
            return motor_hardware::HardwareConfig{
                .direction =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOA,
                     .pin = GPIO_PIN_7,
                     .active_setting = GPIO_PIN_SET},
                .step =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOB,
                     .pin = GPIO_PIN_10,
                     .active_setting = GPIO_PIN_SET},
                .enable =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOD,
                     .pin = GPIO_PIN_2,
                     .active_setting = GPIO_PIN_SET},
                .limit_switch =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_2,
                     .active_setting = GPIO_PIN_SET},
                // LED PIN C11, active setting low
                .led = {},
            };
        case PipetteAxisType::linear_motor_low_throughput:
        default:
            return motor_hardware::HardwareConfig{
                .direction =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_3,
                     .active_setting = GPIO_PIN_SET},
                .step =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_7,
                     .active_setting = GPIO_PIN_SET},
                .enable =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_8,
                     .active_setting = GPIO_PIN_SET},
                .limit_switch =
                    {// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                     .port = GPIOC,
                     .pin = GPIO_PIN_2,
                     .active_setting = GPIO_PIN_SET},
                // LED PIN C11, active setting low
                .led = {},
            };
    }
}

auto interfaces::driver_config(PipetteType pipette)
    -> interfaces::PipetteDriverHardware {
    if (pipette == NINETY_SIX_CHANNEL) {
        return interfaces::PipetteDriverHardware{
            .right_gear_motor = tmc2130_driver_config_by_axis(
                PipetteAxisType::right_gear_motor),
            .left_gear_motor =
                tmc2130_driver_config_by_axis(PipetteAxisType::left_gear_motor),
            .high_throughput_motor = tmc2160_driver_config_by_axis(),
        };
    }
    return interfaces::PipetteDriverHardware{
        .low_throughput_motor = tmc2130_driver_config_by_axis(
            PipetteAxisType::linear_motor_low_throughput)};
}

auto interfaces::hardware_config(PipetteType pipette)
    -> interfaces::PipetteMotorHardware {
    if (pipette == NINETY_SIX_CHANNEL) {
        return interfaces::PipetteMotorHardware{
            .right_gear_motor =
                hardware_config_by_axis(PipetteAxisType::right_gear_motor),
            .left_gear_motor =
                hardware_config_by_axis(PipetteAxisType::left_gear_motor),
            .high_throughput_motor = hardware_config_by_axis(
                PipetteAxisType::linear_motor_high_throughput),
        };
    }
    return interfaces::PipetteMotorHardware{
        .low_throughput_motor = hardware_config_by_axis(
            PipetteAxisType::linear_motor_low_throughput)};
}
