#include "common/core/tool_detection.hpp"

#include <array>
#include <span>

using namespace tool_detection;

constexpr static auto pipette_z_bounds =
    ToolCheckBounds{.upper = 1500, .lower = 201};

constexpr static auto pipette_a_bounds =
    ToolCheckBounds{.upper = 3000, .lower = 1500};

constexpr static auto nothing_connected_z_bounds =
    ToolCheckBounds{.upper = 200, .lower = 0};

constexpr static auto nothing_connected_a_bounds =
    ToolCheckBounds{.upper = 200, .lower = 5};

// revisit these, not sure if EE has a calculation for gripper carrier bounds
constexpr static auto nothing_connected_gripper_bounds =
    ToolCheckBounds{.upper = 54, .lower = 0};

constexpr static auto gripper_bounds =
    ToolCheckBounds{.upper = 3000, .lower = 55};

constexpr static auto failure_bounds =
    ToolCheckBounds{.upper = 4095, .lower = 3001};

static std::array tool_list(std::to_array<Tool>({
    Tool{.tool_type = can::ids::ToolType::pipette,
         .tool_carrier = Z_CARRIER,
         .bounds = pipette_z_bounds},
    Tool{.tool_type = can::ids::ToolType::nothing_attached,
         .tool_carrier = Z_CARRIER,
         .bounds = nothing_connected_z_bounds},
    Tool{.tool_type = can::ids::ToolType::pipette,
         .tool_carrier = A_CARRIER,
         .bounds = pipette_a_bounds},
    Tool{.tool_type = can::ids::ToolType::nothing_attached,
         .tool_carrier = A_CARRIER,
         .bounds = nothing_connected_a_bounds},
    Tool{.tool_type = can::ids::ToolType::gripper,
         .tool_carrier = GRIPPER_CARRIER,
         .bounds = gripper_bounds},
    Tool{.tool_type = can::ids::ToolType::nothing_attached,
         .tool_carrier = GRIPPER_CARRIER,
         .bounds = nothing_connected_gripper_bounds},
    Tool{.tool_type = can::ids::ToolType::tool_error,
         .tool_carrier = UNKNOWN,
         .bounds = failure_bounds},
}));

std::span<Tool> tool_detection::lookup_table() { return std::span(tool_list); }
