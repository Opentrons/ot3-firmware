

#include "head/core/tool_list.hpp"
using namespace ot3_tool_list;

std::array<Tool, 8> _list{{
    {.tool_type = can_ids::ToolType::pipette_384_chan,
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
}};
auto ot3_tool_list::get_tool_list() -> const std::array<Tool, 8>& {
    return _list;
}