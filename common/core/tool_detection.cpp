#include "common/core/tool_detection.hpp"

#include <array>
#include <span>

using namespace tool_detection;

constexpr static auto pipette_384_chan_z_bounds =
    ToolCheckBounds{.upper = 1883, .lower = 1850};

constexpr static auto pipette_96_chan_z_bounds =
    ToolCheckBounds{.upper = 1417, .lower = 1400};

constexpr static auto pipette_single_chan_z_bounds =
    ToolCheckBounds{.upper = 460, .lower = 444};

constexpr static auto pipette_multiple_chan_z_bounds =
    ToolCheckBounds{.upper = 945, .lower = 918};

constexpr static auto pipette_single_chan_a_bounds =
    ToolCheckBounds{.upper = 2389, .lower = 2362};

constexpr static auto pipette_multiple_chan_a_bounds =
    ToolCheckBounds{.upper = 2859, .lower = 2844};

constexpr static auto nothing_connected_z_bounds =
    ToolCheckBounds{.upper = 3, .lower = 1};

constexpr static auto nothing_connected_a_bounds =
    ToolCheckBounds{.upper = 52, .lower = 15};

// revisit these, not sure if EE has a calculation for gripper carrier bounds
constexpr static auto nothing_connected_gripper_bounds =
    ToolCheckBounds{.upper = 54, .lower = 16};

constexpr static auto gripper_bounds =
    ToolCheckBounds{.upper = 999, .lower = 536};

constexpr static auto undefined_bounds =
    ToolCheckBounds{.upper = 0, .lower = 0};

static std::array tool_list(std::to_array<Tool>(
    {Tool{.tool_type = can_ids::ToolType::pipette_384_chan,
          .tool_carrier = Z_CARRIER,
          .bounds = pipette_384_chan_z_bounds},
     Tool{.tool_type = can_ids::ToolType::pipette_96_chan,
          .tool_carrier = Z_CARRIER,
          .bounds = pipette_96_chan_z_bounds},
     Tool{.tool_type = can_ids::ToolType::pipette_single_chan,
          .tool_carrier = Z_CARRIER,
          .bounds = pipette_single_chan_z_bounds},
     Tool{.tool_type = can_ids::ToolType::pipette_multi_chan,
          .tool_carrier = Z_CARRIER,
          .bounds = pipette_multiple_chan_z_bounds},
     Tool{.tool_type = can_ids::ToolType::nothing_attached,
          .tool_carrier = Z_CARRIER,
          .bounds = nothing_connected_z_bounds},
     Tool{.tool_type = can_ids::ToolType::pipette_single_chan,
          .tool_carrier = A_CARRIER,
          .bounds = pipette_single_chan_a_bounds},
     Tool{.tool_type = can_ids::ToolType::pipette_multi_chan,
          .tool_carrier = A_CARRIER,
          .bounds = pipette_multiple_chan_a_bounds},
     Tool{.tool_type = can_ids::ToolType::nothing_attached,
          .tool_carrier = A_CARRIER,
          .bounds = nothing_connected_a_bounds},
     Tool{.tool_type = can_ids::ToolType::gripper,
          .tool_carrier = GRIPPER_CARRIER,
          .bounds = gripper_bounds},
     Tool{.tool_type = can_ids::ToolType::nothing_attached,
          .tool_carrier = GRIPPER_CARRIER,
          .bounds = nothing_connected_gripper_bounds}}));

std::span<Tool> tool_detection::lookup_table() { return std::span(tool_list); }
