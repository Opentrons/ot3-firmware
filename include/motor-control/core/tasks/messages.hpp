#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/usage_messages.hpp"

namespace motor_control_task_messages {

#ifdef USE_PRESSURE_MOVE
using MotionControlTaskMessage = std::variant<
    std::monostate, can::messages::AddLinearMoveRequest,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::StopRequest,
    can::messages::MotorPositionRequest, can::messages::ReadLimitSwitchRequest,
    can::messages::HomeRequest,
    can::messages::UpdateMotorPositionEstimationRequest,
    can::messages::GetMotorUsageRequest, can::messages::AddSensorMoveRequest>;

using MoveGroupTaskMessage =
    std::variant<std::monostate, can::messages::AddLinearMoveRequest,
                 can::messages::ClearAllMoveGroupsRequest,
                 can::messages::ExecuteMoveGroupRequest,
                 can::messages::GetMoveGroupRequest, can::messages::HomeRequest,
                 can::messages::StopRequest,
                 can::messages::AddSensorMoveRequest>;
#else
using MotionControlTaskMessage = std::variant<
    std::monostate, can::messages::AddLinearMoveRequest,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::StopRequest,
    can::messages::MotorPositionRequest, can::messages::ReadLimitSwitchRequest,
    can::messages::HomeRequest,
    can::messages::UpdateMotorPositionEstimationRequest,
    can::messages::GetMotorUsageRequest>;

using MoveGroupTaskMessage =
    std::variant<std::monostate, can::messages::AddLinearMoveRequest,
                 can::messages::ClearAllMoveGroupsRequest,
                 can::messages::ExecuteMoveGroupRequest,
                 can::messages::GetMoveGroupRequest, can::messages::HomeRequest,
                 can::messages::StopRequest>;
#endif

using MotorDriverTaskMessage =
    std::variant<std::monostate, can::messages::ReadMotorDriverRegister,
                 can::messages::WriteMotorDriverRegister,
                 can::messages::WriteMotorCurrentRequest>;

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
    can::messages::GetMotorUsageRequest, can::messages::GripperJawStateRequest,
    can::messages::SetGripperJawHoldoffRequest,
    can::messages::GripperJawHoldoffRequest>;

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
