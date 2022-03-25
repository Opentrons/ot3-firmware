#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace motor_control_task_messages {

using MotionControlTaskMessage = std::variant<
    std::monostate, can_messages::AddLinearMoveRequest,
    can_messages::DisableMotorRequest, can_messages::EnableMotorRequest,
    can_messages::GetMotionConstraintsRequest,
    can_messages::SetMotionConstraints, can_messages::StopRequest,
    can_messages::ReadLimitSwitchRequest, can_messages::HomeRequest>;

using MotorDriverTaskMessage =
    std::variant<std::monostate, can_messages::ReadMotorDriverRegister,
                 can_messages::SetupRequest,
                 can_messages::WriteMotorDriverRegister,
                 can_messages::WriteMotorCurrentRequest>;

using MoveGroupTaskMessage =
    std::variant<std::monostate, can_messages::AddLinearMoveRequest,
                 can_messages::ClearAllMoveGroupsRequest,
                 can_messages::ExecuteMoveGroupRequest,
                 can_messages::GetMoveGroupRequest, can_messages::HomeRequest>;

using MoveStatusReporterTaskMessage = motor_messages::Ack;

using BrushedMotorDriverTaskMessage =
    std::variant<std::monostate, can_messages::SetupRequest,
                 can_messages::SetBrushedMotorVrefRequest,
                 can_messages::SetBrushedMotorPwmRequest>;

}  // namespace motor_control_task_messages