#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/usage_messages.hpp"

namespace motor_control_task_messages {

using MotionControlTaskMessage = std::variant<
    std::monostate, can::messages::AddLinearMoveRequest,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::StopRequest,
    can::messages::MotorPositionRequest, can::messages::ReadLimitSwitchRequest,
    can::messages::HomeRequest,
    can::messages::UpdateMotorPositionEstimationRequest,
    can::messages::GetMotorUsageRequest,
    can::messages::RouteMotorDriverInterrupt,
    can::messages::MotorDriverErrorEncountered,
    can::messages::ResetMotorDriverErrorHandling,
    can::messages::DebounceMotorDriverError>;

using MotorDriverTaskMessage =
    std::variant<std::monostate, can::messages::ReadMotorDriverRegister,
                 can::messages::WriteMotorDriverRegister,
                 can::messages::WriteMotorCurrentRequest,
                 can::messages::ReadMotorDriverErrorStatus>;

using MoveGroupTaskMessage =
    std::variant<std::monostate, can::messages::AddLinearMoveRequest,
                 can::messages::ClearAllMoveGroupsRequest,
                 can::messages::ExecuteMoveGroupRequest,
                 can::messages::GetMoveGroupRequest, can::messages::HomeRequest,
                 can::messages::StopRequest>;

using MoveStatusReporterTaskMessage = std::variant<
    std::monostate, motor_messages::Ack, motor_messages::UpdatePositionResponse,
    can::messages::ErrorMessage, can::messages::StopRequest,
    usage_messages::IncreaseForceTimeUsage, usage_messages::IncreaseErrorCount>;

using BrushedMotorDriverTaskMessage =
    std::variant<std::monostate, can::messages::SetBrushedMotorVrefRequest,
                 can::messages::SetBrushedMotorPwmRequest,
                 can::messages::BrushedMotorConfRequest>;

using BrushedMotionControllerTaskMessage = std::variant<
    std::monostate, can::messages::DisableMotorRequest,
    can::messages::EnableMotorRequest, can::messages::GripperGripRequest,
    can::messages::GripperHomeRequest,
    can::messages::AddBrushedLinearMoveRequest, can::messages::StopRequest,
    can::messages::ReadLimitSwitchRequest, can::messages::MotorPositionRequest,
    can::messages::SetGripperErrorToleranceRequest,
    can::messages::GetMotorUsageRequest, can::messages::GripperJawStateRequest>;

using BrushedMoveGroupTaskMessage = std::variant<
    std::monostate, can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::GripperGripRequest, can::messages::GripperHomeRequest,
    can::messages::AddBrushedLinearMoveRequest, can::messages::StopRequest>;

using UsageStorageTaskMessage =
    std::variant<std::monostate, usage_messages::IncreaseDistanceUsage,
                 usage_messages::GetUsageRequest,
                 usage_messages::IncreaseForceTimeUsage,
                 usage_messages::IncreaseErrorCount>;

}  // namespace motor_control_task_messages
