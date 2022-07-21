#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace motor_control_task_messages {

using MotionControlTaskMessage = std::variant<
    std::monostate, can::messages::AddLinearMoveRequest,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::StopRequest,
    can::messages::EncoderPositionRequest,
    can::messages::ReadLimitSwitchRequest, can::messages::HomeRequest>;

using MotorDriverTaskMessage =
    std::variant<std::monostate, can::messages::ReadMotorDriverRegister,
                 can::messages::WriteMotorDriverRegister,
                 can::messages::WriteMotorCurrentRequest>;

using MoveGroupTaskMessage =
    std::variant<std::monostate, can::messages::AddLinearMoveRequest,
                 can::messages::ClearAllMoveGroupsRequest,
                 can::messages::ExecuteMoveGroupRequest,
                 can::messages::GetMoveGroupRequest,
                 can::messages::HomeRequest>;

using MoveStatusReporterTaskMessage = motor_messages::Ack;

using BrushedMotorDriverTaskMessage =
    std::variant<std::monostate, can::messages::SetBrushedMotorVrefRequest,
                 can::messages::SetBrushedMotorPwmRequest>;

using BrushedMotionControllerTaskMessage =
    std::variant<std::monostate, can::messages::DisableMotorRequest,
                 can::messages::EnableMotorRequest,
                 can::messages::GripperHomeRequest,
                 can::messages::GripperGripRequest, can::messages::StopRequest,
                 can::messages::ReadLimitSwitchRequest,
                 can::messages::EncoderPositionRequest>;

using BrushedMoveGroupTaskMessage = std::variant<
    std::monostate, can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::GripperGripRequest, can::messages::GripperHomeRequest>;

}  // namespace motor_control_task_messages
