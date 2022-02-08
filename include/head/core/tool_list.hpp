#pragma once
#include <array>
#include <cstdint>

#include "can/core/ids.hpp"
#include "head/core/adc.hpp"

namespace ot3_tool_list {

using namespace can_ids;

struct ToolCheckBounds {
    uint16_t upper;
    uint16_t lower;
};

constexpr auto pipette_384_chan_z_bounds =
    ToolCheckBounds{.upper = 1883, .lower = 1851};

constexpr auto pipette_96_chan_z_bounds =
    ToolCheckBounds{.upper = 1434, .lower = 1400};

constexpr auto pipette_single_chan_z_bounds =
    ToolCheckBounds{.upper = 460, .lower = 444};

constexpr auto pipette_multiple_chan_z_bounds =
    ToolCheckBounds{.upper = 945, .lower = 918};

constexpr auto pipette_single_chan_a_bounds =
    ToolCheckBounds{.upper = 2389, .lower = 2362};

constexpr auto pipette_multiple_chan_a_bounds =
    ToolCheckBounds{.upper = 2860, .lower = 2844};

constexpr auto nothing_connected_z_bounds =
    ToolCheckBounds{.upper = 3, .lower = 1};

constexpr auto nothing_connected_a_bounds =
    ToolCheckBounds{.upper = 54, .lower = 16};

// revisit these, not sure if EE has a calculation for gripper carrier bounds
constexpr auto nothing_connected_gripper_bounds =
    ToolCheckBounds{.upper = 54, .lower = 16};

constexpr auto gripper_bounds = ToolCheckBounds{.upper = 999, .lower = 536};

constexpr auto undefined_bounds = ToolCheckBounds{.upper = 0, .lower = 0};

enum Carrier : uint8_t {
    GRIPPER_CARRIER = 0x00,
    Z_CARRIER = 0x01,
    A_CARRIER = 0x02,
};

struct Tool {
    ToolType tool_type;
    Carrier tool_carrier;
    ToolCheckBounds bounds;
    [[nodiscard]] auto within_bounds(uint16_t reading) const -> bool {
        return ((reading >= this->bounds.lower) &&
                (reading < this->bounds.upper));
        return false;
    }
};

using OT3Tools = std::array<Tool, 10>;

auto get_tool_list() -> const OT3Tools&;
struct AttachedTool {
    ToolType z_motor{};
    ToolType a_motor{};
    ToolType gripper{};
    AttachedTool()
        : z_motor(can_ids::ToolType::undefined_tool),
          a_motor(can_ids::ToolType::undefined_tool),
          gripper(can_ids::ToolType::undefined_tool) {}
    AttachedTool(adc::MillivoltsReadings reading, const OT3Tools& arr) {
        for (const auto& element : arr) {
            if (element.within_bounds(reading.z_motor) &&
                (element.tool_carrier == Z_CARRIER)) {
                this->z_motor = element.tool_type;
            }
            if (element.within_bounds(reading.a_motor) &&
                (element.tool_carrier == A_CARRIER)) {
                this->a_motor = element.tool_type;
            }
            if (element.within_bounds(reading.gripper) &&
                (element.tool_carrier == GRIPPER_CARRIER)) {
                this->gripper = element.tool_type;
            }
        }
    }
};

}  // namespace ot3_tool_list