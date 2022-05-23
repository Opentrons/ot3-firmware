#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace pipettes {

namespace task_messages {

namespace motor_control_task_messages {

using MotionControlTaskMessage = std::variant<
    std::monostate,
    can_messages::DisableMotorRequest, can_messages::EnableMotorRequest,
    can_messages::GetMotionConstraintsRequest,
    can_messages::SetMotionConstraints, can_messages::StopRequest,
    can_messages::ReadLimitSwitchRequest, can_messages::TipActionRequest>;

// TODO do we want the ack message to be slightly diff here?
using MoveStatusReporterTaskMessage = motor_messages::Ack;

} // namespace motor_control_task_messages

namespace move_group_task_messages {

using MoveGroupTaskMessage = std::variant<
    std::monostate, can_messages::ClearAllMoveGroupsRequest,
    can_messages::ExecuteMoveGroupRequest,
    can_messages::GetMoveGroupRequest, can_messages::TipActionRequest>;

} // namespace move_group_task_messages
} // namespace task_messages
} // namespace pipettes