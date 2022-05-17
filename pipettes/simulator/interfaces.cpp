#include "pipettes/core/interfaces.hpp"

#define GPIO_C_DEF ((void *)0x1000)

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
                    .cs_pin = 512,
                    .GPIO_handle = GPIO_C_DEF,
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
                    .cs_pin = 1024,
                    .GPIO_handle = GPIO_C_DEF,
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
                    .cs_pin = 64,
                    .GPIO_handle = GPIO_C_DEF,
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
                    .cs_pin = 64,
                    .GPIO_handle = GPIO_C_DEF,
                }};
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