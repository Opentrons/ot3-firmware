#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/usage_messages.hpp"

namespace pipettes {

namespace task_messages {

namespace motor_control_task_messages {

struct RouteMotorDriverInterrupt {
    uint32_t message_index;
    uint8_t debounce_count;
};

using MotionControlTaskMessage = std::variant<
    std::monostate, can::messages::TipActionRequest,
    can::messages::GearDisableMotorRequest,
    can::messages::GearEnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::StopRequest,
    can::messages::ReadLimitSwitchRequest, can::messages::GetMotorUsageRequest,
    RouteMotorDriverInterrupt>;

using MoveStatusReporterTaskMessage =
    std::variant<std::monostate, motor_messages::GearMotorAck,
                 motor_messages::UpdatePositionResponse,
                 can::messages::StopRequest, can::messages::ErrorMessage,
                 usage_messages::IncreaseErrorCount>;

}  // namespace motor_control_task_messages

namespace move_group_task_messages {

using MoveGroupTaskMessage =
    std::variant<std::monostate, can::messages::ClearAllMoveGroupsRequest,
                 can::messages::ExecuteMoveGroupRequest,
                 can::messages::GetMoveGroupRequest,
                 can::messages::TipActionRequest, can::messages::StopRequest>;

}  // namespace move_group_task_messages
}  // namespace task_messages
}  // namespace pipettes
