#include "common/core/tool_detection.hpp"

#include <array>
#include <span>

using namespace tool_detection;

constexpr static auto pipette_chan_z_bounds =
    ToolCheckBounds{.upper = 580, .lower = 540};

constexpr static auto pipette_chan_a_bounds =
    ToolCheckBounds{.upper = 3050, .lower = 2900};

constexpr static auto nothing_connected_z_bounds =
    ToolCheckBounds{.upper = 5, .lower = 0};

constexpr static auto nothing_connected_a_bounds =
    ToolCheckBounds{.upper = 50, .lower = 5};

// revisit these, not sure if EE has a calculation for gripper carrier bounds
constexpr static auto nothing_connected_gripper_bounds =
    ToolCheckBounds{.upper = 54, .lower = 0};

constexpr static auto gripper_bounds =
    ToolCheckBounds{.upper = 3000, .lower = 55};

constexpr static auto undefined_bounds =
    ToolCheckBounds{.upper = 4000, .lower = 3100};

constexpr static auto failure_bounds =
    ToolCheckBounds{.upper = 4095, .lower = 4000};

static std::array tool_list(std::to_array<Tool>({
    Tool{.tool_type = can::ids::ToolType::pipette,
         .tool_carrier = Z_CARRIER,
         .bounds = pipette_chan_z_bounds},
    Tool{.tool_type = can::ids::ToolType::nothing_attached,
         .tool_carrier = Z_CARRIER,
         .bounds = nothing_connected_z_bounds},
    Tool{.tool_type = can::ids::ToolType::tool_error,
         .tool_carrier = Z_CARRIER,
         .bounds = failure_bounds},
    Tool{.tool_type = can::ids::ToolType::pipette,
         .tool_carrier = A_CARRIER,
         .bounds = pipette_chan_a_bounds},
    Tool{.tool_type = can::ids::ToolType::nothing_attached,
         .tool_carrier = A_CARRIER,
         .bounds = nothing_connected_a_bounds},
    Tool{.tool_type = can::ids::ToolType::tool_error,
         .tool_carrier = A_CARRIER,
         .bounds = failure_bounds},
    Tool{.tool_type = can::ids::ToolType::gripper,
         .tool_carrier = GRIPPER_CARRIER,
         .bounds = gripper_bounds},
    Tool{.tool_type = can::ids::ToolType::nothing_attached,
         .tool_carrier = GRIPPER_CARRIER,
         .bounds = nothing_connected_gripper_bounds},
    Tool{.tool_type = can::ids::ToolType::tool_error,
         .tool_carrier = GRIPPER_CARRIER,
         .bounds = failure_bounds},
    Tool{.tool_type = can::ids::ToolType::undefined_tool,
         .tool_carrier = UNKNOWN,
         .bounds = undefined_bounds},
}));

std::span<Tool> tool_detection::lookup_table() { return std::span(tool_list); }
