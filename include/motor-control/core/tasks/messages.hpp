#pragma once

#include "can/core/messages.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/usage_messages.hpp"

namespace motor_control_task_messages {

/*enum class MessageId {
    route_motor_driver_interrupt = 0x0,
    motor_driver_error_encountered = 0x1,
    reset_motor_driver_error_handling = 0x2,
};

template <MessageId MId>
struct BaseMessage {
    static const auto id = MId;
    auto operator==(const BaseMessage& other) const -> bool = default;
};

template <MessageId MId>
struct Empty : BaseMessage<MId> {
    uint32_t message_index;
    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> Empty {
        uint32_t msg_ind = 0;
        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        return Empty{.message_index = msg_ind};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        return iter - body;
    }

    auto operator==(const Empty&) const -> bool = default;
};

using RouteMotorDriverInterrupt = Empty<MessageId::route_motor_driver_interrupt>;

using MotorDriverErrorEncountered = Empty<MessageId::motor_driver_error_encountered>;

using ResetMotorDriverErrorHandling = Empty<MessageId::reset_motor_driver_error_handling>;

struct RouteMotorDriverInterrupt {
    uint32_t message_index;
    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> RouteMotorDriverInterrupt {
        uint32_t msg_ind = 0;
        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        return RouteMotorDriverInterrupt{.message_index = msg_ind};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        return iter - body;
    }

    auto operator==(const RouteMotorDriverInterrupt&) const -> bool = default;
};

struct MotorDriverErrorEncountered {
    uint32_t message_index;
    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> MotorDriverErrorEncountered {
        uint32_t msg_ind = 0;
        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        return MotorDriverErrorEncountered{.message_index = msg_ind};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        return iter - body;
    }

    auto operator==(const MotorDriverErrorEncountered&) const -> bool = default;
};

struct ResetMotorDriverErrorHandling {
    uint32_t message_index;
    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ResetMotorDriverErrorHandling {
        uint32_t msg_ind = 0;
        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        return ResetMotorDriverErrorHandling{.message_index = msg_ind};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        return iter - body;
    }

    auto operator==(const ResetMotorDriverErrorHandling&) const -> bool = default;
};*/
// needs a member 'id'. This seems to just be becoming a CAN msg!

struct RouteMotorDriverInterrupt {
    uint32_t message_index;
};

using MotionControlTaskMessage = std::variant<
    std::monostate, can::messages::AddLinearMoveRequest,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::StopRequest,
    can::messages::MotorPositionRequest, can::messages::ReadLimitSwitchRequest,
    can::messages::HomeRequest,
    can::messages::UpdateMotorPositionEstimationRequest,
    can::messages::GetMotorUsageRequest, RouteMotorDriverInterrupt>;

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
