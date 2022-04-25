#include "pipettes/core/interfaces.hpp"

auto interfaces::driver_config_by_axis(PipetteType which)
    -> tmc2130::configs::TMC2130DriverConfig {
    switch (which) {
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
                    .cs_pin = 0,
                    .GPIO_handle = 0,
                }};
    }
}