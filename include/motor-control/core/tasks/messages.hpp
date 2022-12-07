#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace motor_control_task_messages {

using MotionControlTaskMessage = std::variant<
    std::monostate, can::messages::AddLinearMoveRequest,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::StopRequest,
    can::messages::MotorPositionRequest, can::messages::ReadLimitSwitchRequest,
    can::messages::HomeRequest>;

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

using MoveStatusReporterTaskMessage =
    std::variant<std::monostate, motor_messages::Ack,
                 can::messages::ErrorMessage>;

using BrushedMotorDriverTaskMessage =
    std::variant<std::monostate, can::messages::SetBrushedMotorVrefRequest,
                 can::messages::SetBrushedMotorPwmRequest,
                 can::messages::BrushedMotorConfRequest>;

using BrushedMotionControllerTaskMessage = std::variant<
    std::monostate, can::messages::DisableMotorRequest,
    can::messages::EnableMotorRequest, can::messages::GripperGripRequest,
    can::messages::GripperHomeRequest,
    can::messages::AddBrushedLinearMoveRequest, can::messages::StopRequest,
    can::messages::ReadLimitSwitchRequest, can::messages::MotorPositionRequest>;

using BrushedMoveGroupTaskMessage = std::variant<
    std::monostate, can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::GripperGripRequest, can::messages::GripperHomeRequest,
    can::messages::AddBrushedLinearMoveRequest>;

}  // namespace motor_control_task_messages
