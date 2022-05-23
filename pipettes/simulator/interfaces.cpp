#include "pipettes/core/interfaces.hpp"

int dummy_gpio = 0x1000;
#define GPIO_C_DEF ((void *)(&dummy_gpio))

auto interfaces::driver_config_by_axis(TMC2160PipetteAxis which)
    -> tmc2160::configs::TMC2160DriverConfig {
    switch (which) {
        case TMC2160PipetteAxis::linear_motor:
        default:
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
}

auto interfaces::driver_config_by_axis(TMC2130PipetteAxis which)
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
        case TMC2130PipetteAxis::left_gear_motor:
            return tmc2130_conf;
        case TMC2130PipetteAxis::right_gear_motor:
            tmc2130_conf.chip_select = {
                .cs_pin = 64,
                .GPIO_handle = GPIO_C_DEF,
            };
            return tmc2130_conf;
        case TMC2130PipetteAxis::linear_motor:
        default:
            tmc2130_conf.chip_select = {
                .cs_pin = 64,
                .GPIO_handle = GPIO_C_DEF,
            };
            return tmc2130_conf;
    }
}

template <>
auto interfaces::motor_configurations<PipetteType::SINGLE_CHANNEL>()
    -> interfaces::LowThroughputMotorConfigurations {
    auto configs = interfaces::LowThroughputPipetteDriverHardware{
        .linear_motor =
            driver_config_by_axis(TMC2130PipetteAxis::linear_motor)};
    auto pins = interfaces::LowThroughputPipetteMotorHardware{};
    return interfaces::LowThroughputMotorConfigurations{
        .hardware_pins = pins, .driver_configs = configs};
}

template <>
auto interfaces::motor_configurations<PipetteType::EIGHT_CHANNEL>()
    -> interfaces::LowThroughputMotorConfigurations {
    auto configs = interfaces::LowThroughputPipetteDriverHardware{
        .linear_motor =
            driver_config_by_axis(TMC2130PipetteAxis::linear_motor)};
    auto pins = interfaces::LowThroughputPipetteMotorHardware{};
    return interfaces::LowThroughputMotorConfigurations{
        .hardware_pins = pins, .driver_configs = configs};
}

template <>
auto interfaces::motor_configurations<PipetteType::NINETY_SIX_CHANNEL>()
    -> interfaces::HighThroughputMotorConfigurations {
    auto configs = interfaces::HighThroughputPipetteDriverHardware{
        .right_gear_motor =
            driver_config_by_axis(TMC2130PipetteAxis::right_gear_motor),
        .left_gear_motor =
            driver_config_by_axis(TMC2130PipetteAxis::left_gear_motor),
        .linear_motor =
            driver_config_by_axis(TMC2160PipetteAxis::linear_motor)};
    auto pins = interfaces::HighThroughputPipetteMotorHardware{};
    return interfaces::HighThroughputMotorConfigurations{
        .hardware_pins = pins, .driver_configs = configs};
}

template <>
auto interfaces::motor_configurations<PipetteType::THREE_EIGHTY_FOUR_CHANNEL>()
    -> interfaces::HighThroughputMotorConfigurations {
    auto configs = interfaces::HighThroughputPipetteDriverHardware{
        .right_gear_motor =
            driver_config_by_axis(TMC2130PipetteAxis::right_gear_motor),
        .left_gear_motor =
            driver_config_by_axis(TMC2130PipetteAxis::left_gear_motor),
        .linear_motor =
            driver_config_by_axis(TMC2160PipetteAxis::linear_motor)};
    auto pins = interfaces::HighThroughputPipetteMotorHardware{};
    return interfaces::HighThroughputMotorConfigurations{
        .hardware_pins = pins, .driver_configs = configs};
}
