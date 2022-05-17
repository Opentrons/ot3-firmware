#include "pipettes/core/interfaces.hpp"

int dummy_gpio = 0x1000;
#define GPIO_C_DEF ((void *)(&dummy_gpio))

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
            .cs_pin = 64,
            .GPIO_handle = GPIO_C_DEF,
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
            .cs_pin = 512,
            .GPIO_handle = GPIO_C_DEF,
        }};
    switch (which) {
        case PipetteAxisType::left_gear_motor:
            return tmc2130_conf;
        case PipetteAxisType::right_gear_motor:
            tmc2130_conf.chip_select = {
                .cs_pin = 64,
                .GPIO_handle = GPIO_C_DEF,
            };
            return tmc2130_conf;
        case PipetteAxisType::linear_motor_low_throughput:
        default:
            tmc2130_conf.chip_select = {
                .cs_pin = 64,
                .GPIO_handle = GPIO_C_DEF,
            };
            return tmc2130_conf;
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