#include "gantry/core/utils.hpp"

auto utils::get_node_id_by_axis(enum GantryAxisType which) -> can_ids::NodeId {
    switch (which) {
        case GantryAxisType::gantry_x:
            return can_ids::NodeId::gantry_x;
        case GantryAxisType::gantry_y:
            return can_ids::NodeId::gantry_y;
    }
}

auto utils::get_node_id() -> can_ids::NodeId {
    return get_node_id_by_axis(get_axis_type());
}

auto utils::register_config_by_axis(enum GantryAxisType which)
    -> motor_driver_config::RegisterConfig {
    switch (which) {
        case GantryAxisType::gantry_x:
            return motor_driver_config::RegisterConfig{.gconf = 0x04,
                                                       .ihold_irun = 0x71002,
                                                       .chopconf = 0x101D5,
                                                       .thigh = 0xFFFFF,
                                                       .coolconf = 0x60000};

        case GantryAxisType::gantry_y:
            return motor_driver_config::RegisterConfig{.gconf = 0x04,
                                                       .ihold_irun = 0x71002,
                                                       .chopconf = 0x101D5,
                                                       .thigh = 0xFFFFF,
                                                       .coolconf = 0x60000};
    }
}

auto utils::register_config() -> motor_driver_config::RegisterConfig {
    return register_config_by_axis(get_axis_type());
}
