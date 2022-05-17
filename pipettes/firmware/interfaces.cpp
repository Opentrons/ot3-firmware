#include "pipettes/core/interfaces.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic pop

auto interfaces::driver_config_by_axis(PipetteAxisType which)
    -> driver_utils::DriverConfigType {
    switch (which) {
        case PipetteAxisType::left_gear_motor:
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
                    .cs_pin = GPIO_PIN_9,
                    .GPIO_handle = GPIOC,
                }};
        case PipetteAxisType::right_gear_motor:
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
                    .cs_pin = GPIO_PIN_10,
                    .GPIO_handle = GPIOC,
                }};
        case PipetteAxisType::linear_motor_high_throughput:
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
                    .GPIO_handle = GPIOC,
                }};
        case PipetteAxisType::linear_motor_low_throughput:
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
                    .cs_pin = GPIO_PIN_6,
                    .GPIO_handle = GPIOC,
                }};
    }
}

auto interfaces::hardware_config_by_axis(PipetteAxisType which)
    -> motor_hardware::HardwareConfig {
    switch (which) {
        case PipetteAxisType::right_gear_motor:
            return motor_hardware::HardwareConfig{
                .direction = {.port = GPIOC,
                              .pin = GPIO_PIN_13,
                              .active_setting = GPIO_PIN_SET},
                .step = {.port = GPIOB,
                         .pin = GPIO_PIN_8,
                         .active_setting = GPIO_PIN_SET},
                .enable = {.port = GPIOD,
                           .pin = GPIO_PIN_2,
                           .active_setting = GPIO_PIN_SET},
                .limit_switch = {.port = GPIOC,
                                 .pin = GPIO_PIN_14,
                                 .active_setting = GPIO_PIN_SET},
                .led = {.port = GPIOC,
                        .pin = GPIO_PIN_11,
                        .active_setting = GPIO_PIN_RESET},
            };
        case PipetteAxisType::left_gear_motor:
            return motor_hardware::HardwareConfig{
                .direction = {.port = GPIOC,
                              .pin = GPIO_PIN_7,
                              .active_setting = GPIO_PIN_SET},
                .step = {.port = GPIOC,
                         .pin = GPIO_PIN_8,
                         .active_setting = GPIO_PIN_SET},
                .enable = {.port = GPIOD,
                           .pin = GPIO_PIN_2,
                           .active_setting = GPIO_PIN_SET},
                .limit_switch = {.port = GPIOA,
                                 .pin = GPIO_PIN_10,
                                 .active_setting = GPIO_PIN_SET},
                .led = {.port = GPIOC,
                        .pin = GPIO_PIN_11,
                        .active_setting = GPIO_PIN_RESET},
            };
        case PipetteAxisType::linear_motor_high_throughput:
            return motor_hardware::HardwareConfig{
                .direction = {.port = GPIOA,
                              .pin = GPIO_PIN_7,
                              .active_setting = GPIO_PIN_SET},
                .step = {.port = GPIOB,
                         .pin = GPIO_PIN_10,
                         .active_setting = GPIO_PIN_SET},
                .enable = {.port = GPIOD,
                           .pin = GPIO_PIN_2,
                           .active_setting = GPIO_PIN_SET},
                .limit_switch = {.port = GPIOC,
                                 .pin = GPIO_PIN_2,
                                 .active_setting = GPIO_PIN_SET},
                .led = {.port = GPIOC,
                        .pin = GPIO_PIN_11,
                        .active_setting = GPIO_PIN_RESET},
            };
        case PipetteAxisType::linear_motor_low_throughput:
        default:
            return motor_hardware::HardwareConfig{
                .direction = {.port = GPIOC,
                              .pin = GPIO_PIN_3,
                              .active_setting = GPIO_PIN_SET},
                .step = {.port = GPIOC,
                         .pin = GPIO_PIN_7,
                         .active_setting = GPIO_PIN_SET},
                .enable = {.port = GPIOC,
                           .pin = GPIO_PIN_8,
                           .active_setting = GPIO_PIN_SET},
                .limit_switch = {.port = GPIOC,
                                 .pin = GPIO_PIN_2,
                                 .active_setting = GPIO_PIN_SET},
                .led = {.port = GPIOA,
                        .pin = GPIO_PIN_8,
                        .active_setting = GPIO_PIN_RESET},
            };
    }
}

auto interfaces::driver_config(PipetteType pipette)
    -> interfaces::PipetteDriverHardware {
    if (pipette == NINETY_SIX_CHANNEL) {
        return interfaces::PipetteDriverHardware{
            .right_gear_motor = static_cast<interfaces::TMC2130DriverConfig>(
                driver_config_by_axis(PipetteAxisType::right_gear_motor)),
            .left_gear_motor = static_cast<interfaces::TMC2130DriverConfig>(
                driver_config_by_axis(PipetteAxisType::left_gear_motor)),
            .high_throughput_motor =
                static_cast<interfaces::TMC2160DriverConfig>(
                    driver_config_by_axis(
                        PipetteAxisType::linear_motor_high_throughput)),
        };
    } else {
        return interfaces::PipetteDriverHardware{
            .low_throughput_motor =
                static_cast<interfaces::TMC2130DriverConfig>(
                    driver_config_by_axis(
                        PipetteAxisType::linear_motor_low_throughput))};
    }
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
    } else {
        return interfaces::PipetteMotorHardware{
            .low_throughput_motor = hardware_config_by_axis(
                PipetteAxisType::linear_motor_low_throughput)};
    }
}
