#pragma once
#include <array>
#include <cstdint>

#include "can/core/ids.hpp"
#include "common/core/tool_detection.hpp"
#include "head/core/adc.hpp"

namespace attached_tools {

using namespace tool_detection;

struct AttachedTools {
    can::ids::ToolType z_motor = can::ids::ToolType::nothing_attached;
    can::ids::ToolType a_motor = can::ids::ToolType::nothing_attached;
    can::ids::ToolType gripper = can::ids::ToolType::nothing_attached;
    AttachedTools() = default;
    AttachedTools(adc::MillivoltsReadings reading)
        : z_motor(tooltype_from_reading(
              reading.z_motor, lookup_table_filtered(Carrier::Z_CARRIER))),
          a_motor(tooltype_from_reading(
              reading.a_motor, lookup_table_filtered(Carrier::A_CARRIER))),
          gripper(tooltype_from_reading(
              reading.gripper,
              lookup_table_filtered(Carrier::GRIPPER_CARRIER))) {}

    AttachedTools(adc::MillivoltsReadings reading, std::span<Tool> arr)
        : z_motor(tooltype_from_reading(reading.z_motor, arr)),
          a_motor(tooltype_from_reading(reading.a_motor, arr)),
          gripper(tooltype_from_reading(reading.gripper, arr)) {
        {}
    }
    constexpr AttachedTools(can::ids::ToolType z_motor_in,
                            can::ids::ToolType a_motor_in,
                            can::ids::ToolType gripper_in)
        : z_motor(z_motor_in), a_motor(a_motor_in), gripper(gripper_in) {}
};

}  // namespace attached_tools
