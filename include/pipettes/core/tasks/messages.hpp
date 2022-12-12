#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace pipettes {

namespace task_messages {

namespace motor_control_task_messages {

using MotionControlTaskMessage = std::variant<
    std::monostate, can::messages::DisableMotorRequest,
    can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::StopRequest,
    can::messages::ReadLimitSwitchRequest, can::messages::TipActionRequest>;

// TODO do we want the ack message to be slightly diff here?
using MoveStatusReporterTaskMessage =
    std::variant<motor_messages::GearMotorAck,
                 motor_messages::UpdatePositionResponse>;

}  // namespace motor_control_task_messages

namespace move_group_task_messages {

using MoveGroupTaskMessage =
    std::variant<std::monostate, can::messages::ClearAllMoveGroupsRequest,
                 can::messages::ExecuteMoveGroupRequest,
                 can::messages::GetMoveGroupRequest,
                 can::messages::TipActionRequest>;

}  // namespace move_group_task_messages
}  // namespace task_messages
}  // namespace pipettes
