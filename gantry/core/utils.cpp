#include "gantry/core/utils.hpp"

#include <cstdlib>

auto utils::get_node_id_by_axis(enum GantryAxisType which) -> can_ids::NodeId {
    switch (which) {
        case GantryAxisType::gantry_x:
            return can_ids::NodeId::gantry_x;
        case GantryAxisType::gantry_y:
            return can_ids::NodeId::gantry_y;
    }
    std::abort();
}

auto utils::get_node_id() -> can_ids::NodeId {
    return get_node_id_by_axis(get_axis_type());
}

auto utils::register_config_by_axis(enum GantryAxisType which)
    -> tmc2130::TMC2130RegisterMap {
    switch (which) {
        case GantryAxisType::gantry_x:
            return tmc2130::TMC2130RegisterMap{
                .gconfig = {.en_pwm_mode = 1},
                .ihold_irun = {.hold_current = 0x2,
                               .run_current = 0x10,
                               .hold_current_delay = 0x7},
                .thigh = {.threshold = 0xFFFFF},
                .chopconf = {.toff = 0x5,
                             .hstrt = 0x5,
                             .hend = 0x3,
                             .tbl = 0x2,
                             .mres = 0x4},
                .coolconf = {.sgt = 0x6}};

        case GantryAxisType::gantry_y:
            return tmc2130::TMC2130RegisterMap{
                .gconfig = {.en_pwm_mode = 1},
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
                             .mres = 0x4},
                .coolconf = {.sgt = 0x6}};
    }
    std::abort();
}

auto utils::register_config() -> tmc2130::TMC2130RegisterMap {
    return register_config_by_axis(get_axis_type());
}
