#include "pipettes/core/motor_configurations.hpp"

int dummy_gpio = 0x1000;
#define GPIO_C_DEF ((void *)(&dummy_gpio))

auto motor_configs::driver_config_by_axis(TMC2160PipetteAxis which)
    -> tmc2160::configs::TMC2160DriverConfig {
    tmc2160::configs::TMC2160DriverConfig tmc2160_conf{
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
                      .glob_scale = {.global_scaler = 0xA7}},
        .current_config =
            {
                .r_sense = 0.1,
                .v_sf = 0.325,
            },
        .chip_select = {
            .cs_pin = 64,
            .GPIO_handle = GPIO_C_DEF,
        }};
    switch (which) {
        case TMC2160PipetteAxis::left_gear_motor:
            return tmc2160_conf;
        case TMC2160PipetteAxis::right_gear_motor:
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
                    .cs_pin = 64,
                    .GPIO_handle = GPIO_C_DEF,
                }};
    }
}

template <>
auto motor_configs::motor_configurations<PipetteType::SINGLE_CHANNEL>()
    -> motor_configs::LowThroughputMotorConfigurations {
    auto configs = motor_configs::LowThroughputPipetteDriverHardware{
        .linear_motor =
            driver_config_by_axis(TMC2130PipetteAxis::linear_motor)};
    auto pins = motor_configs::LowThroughputPipetteMotorHardware{};
    return motor_configs::LowThroughputMotorConfigurations{
        .hardware_pins = pins, .driver_configs = configs};
}

template <>
auto motor_configs::motor_configurations<PipetteType::EIGHT_CHANNEL>()
    -> motor_configs::LowThroughputMotorConfigurations {
    auto configs = motor_configs::LowThroughputPipetteDriverHardware{
        .linear_motor =
            driver_config_by_axis(TMC2130PipetteAxis::linear_motor)};
    auto pins = motor_configs::LowThroughputPipetteMotorHardware{};
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
    auto pins = motor_configs::HighThroughputPipetteMotorHardware{};
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
    auto pins = motor_configs::HighThroughputPipetteMotorHardware{};
    return motor_configs::HighThroughputMotorConfigurations{
        .hardware_pins = pins, .driver_configs = configs};
}
