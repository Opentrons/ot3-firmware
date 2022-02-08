

#include "head/core/tool_list.hpp"
using namespace ot3_tool_list;

static OT3Tools _list{{{.tool_type = can_ids::ToolType::pipette_384_chan,
                        .tool_carrier = Z_CARRIER,
                        .bounds = pipette_384_chan_z_bounds},
                       {.tool_type = can_ids::ToolType::pipette_96_chan,
                        .tool_carrier = Z_CARRIER,
                        .bounds = pipette_96_chan_z_bounds},
                       {.tool_type = can_ids::ToolType::pipette_single_chan,
                        .tool_carrier = Z_CARRIER,
                        .bounds = pipette_single_chan_z_bounds},
                       {.tool_type = can_ids::ToolType::pipette_multi_chan,
                        .tool_carrier = Z_CARRIER,
                        .bounds = pipette_multiple_chan_z_bounds},
                       {.tool_type = can_ids::ToolType::pipette_single_chan,
                        .tool_carrier = A_CARRIER,
                        .bounds = pipette_single_chan_a_bounds},
                       {.tool_type = can_ids::ToolType::pipette_multi_chan,
                        .tool_carrier = A_CARRIER,
                        .bounds = pipette_multiple_chan_a_bounds},
                       {.tool_type = can_ids::ToolType::gripper,
                        .tool_carrier = GRIPPER_CARRIER,
                        .bounds = gripper_bounds},
                       {.tool_type = can_ids::ToolType::nothing_attached,
                        .tool_carrier = Z_CARRIER,
                        .bounds = nothing_connected_z_bounds},
                       {.tool_type = can_ids::ToolType::nothing_attached,
                        .tool_carrier = A_CARRIER,
                        .bounds = nothing_connected_a_bounds},
                       {.tool_type = can_ids::ToolType::nothing_attached,
                        .tool_carrier = GRIPPER_CARRIER,
                        .bounds = nothing_connected_gripper_bounds}}};
auto ot3_tool_list::get_tool_list() -> const OT3Tools& { return _list; }