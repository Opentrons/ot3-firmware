#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace motor_control_task_messages {


using MotionControlTaskMessage =
    std::variant<std::monostate, can_messages::StopRequest, can_messages::EnableMotorRequest,
                 can_messages::DisableMotorRequest, can_messages::GetMotionConstraintsRequest,
                 can_messages::SetMotionConstraints, can_messages::AddLinearMoveRequest>;


using MotorDriverTaskMessage = std::variant<std::monostate, can_messages::SetupRequest,
                                 can_messages::WriteMotorDriverRegister,
                                 can_messages::ReadMotorDriverRegister>;



using MoveGroupTaskMessage = std::variant<std::monostate, can_messages::AddLinearMoveRequest,
                                 can_messages::GetMoveGroupRequest, can_messages::ClearAllMoveGroupsRequest, can_messages::ExecuteMoveGroupRequest>;


using MoveStatusReporterTaskMessage = motor_messages::Ack;


}