#include "gantry/core/utils.hpp"

auto utils::get_node_id_by_axis(enum GantryAxisType which) -> can_ids::NodeId {
    switch (which) {
        case GantryAxisType::gantry_x:
            return can_ids::NodeId::gantry_x;
        case GantryAxisType::gantry_y:
            return can_ids::NodeId::gantry_y;
        default:
            // Return a bootloader node id for the X gantry
            // if both other cases fail.
            return can_ids::NodeId::gantry_x_bootloader;
    }
}

auto utils::get_node_id() -> can_ids::NodeId {
    return get_node_id_by_axis(get_axis_type());
}

auto utils::linear_motion_sys_config_by_axis(enum GantryAxisType which)
    -> lms::LinearMotionSystemConfig<lms::BeltConfig> {
    switch (which) {
        case GantryAxisType::gantry_x:
            return lms::LinearMotionSystemConfig<lms::BeltConfig>{
                .mech_config = lms::BeltConfig{.pulley_diameter = 12.7},
                .steps_per_rev = 200,
                .microstep = 32,
            };
        case GantryAxisType::gantry_y:
            return lms::LinearMotionSystemConfig<lms::BeltConfig>{
                .mech_config = lms::BeltConfig{.pulley_diameter = 12.7254},
                .steps_per_rev = 200,
                .microstep = 32,
            };
        default:
            return lms::LinearMotionSystemConfig<lms::BeltConfig>{};
    }
}

auto utils::linear_motion_system_config()
    -> lms::LinearMotionSystemConfig<lms::BeltConfig> {
    return linear_motion_sys_config_by_axis(get_axis_type());
}