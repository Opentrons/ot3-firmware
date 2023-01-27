#pragma once
#include <array>
#include <cstdint>

#include "can/core/ids.hpp"

namespace attached_tools {

struct MountPinMeasurements {
    bool left_present;
    bool right_present;
    bool gripper_present;
};

struct AttachedTools {
    can::ids::ToolType z_motor = can::ids::ToolType::nothing_attached;
    can::ids::ToolType a_motor = can::ids::ToolType::nothing_attached;
    can::ids::ToolType gripper = can::ids::ToolType::nothing_attached;
    AttachedTools() = default;
    AttachedTools(MountPinMeasurements reading)
        : z_motor(tooltype_from_present(reading.left_present,
                                        can::ids::ToolType::pipette)),
          a_motor(tooltype_from_present(reading.right_present,
                                        can::ids::ToolType::pipette)),
          gripper(tooltype_from_present(reading.gripper_present,
                                        can::ids::ToolType::gripper)) {}

    constexpr AttachedTools(can::ids::ToolType z_motor_in,
                            can::ids::ToolType a_motor_in,
                            can::ids::ToolType gripper_in)
        : z_motor(z_motor_in), a_motor(a_motor_in), gripper(gripper_in) {}
    static auto tooltype_from_present(bool present,
                                      can::ids::ToolType value_if_present)
        -> can::ids::ToolType {
        return present ? value_if_present
                       : can::ids::ToolType::nothing_attached;
    }
};

}  // namespace attached_tools
