#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "can/core/ids.hpp"
#include "can/core/message_core.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/version.h"
#include "eeprom/core/serial_number.hpp"
#include "eeprom/core/types.hpp"
#include "parse.hpp"

namespace can::messages {

using namespace can::ids;

using stepper_timer_ticks = uint32_t;
using brushed_timer_ticks = uint32_t;
using mm_per_tick = int32_t;
using um_per_tick_sq = int32_t;

/**
 * These types model the messages being sent and received over the can bus.
 *
 * The objects must implement the Parsable concept to deserialize the payload.
 */

template <MessageId MId>
struct BaseMessage {
    /** Satisfy the HasMessageID concept */
    static const auto id = MId;
    auto operator==(const BaseMessage& other) const -> bool = default;
};

/**
 * A message with no payload.
 *
 * @tparam MId
 */
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

using Acknowledgment = Empty<MessageId::acknowledgement>;

/* convience functions for creating responses */
template <can::message_core::HasMessageIndex Request>
static auto ack_from_request(const Request& r) -> Acknowledgment {
    return Acknowledgment{.message_index = r.message_index};
}

template <can::message_core::HasMessageIndex Request,
          can::message_core::HasMessageIndex Resposne>
static auto add_resp_ind(Resposne& resp, const Request& req) -> void {
    resp.message_index = req.message_index;
}

struct ErrorMessage : BaseMessage<MessageId::error_message> {
    uint32_t message_index;
    ErrorSeverity severity;
    ErrorCode error_code;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint16_t>(severity), iter,
                                       limit);
        iter = bit_utils::int_to_bytes(static_cast<uint16_t>(error_code), iter,
                                       limit);
        return iter - body;
    }

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ErrorMessage {
        uint32_t message_index = 0;
        uint16_t severity = 0;
        uint16_t error_code = 0;
        body = bit_utils::bytes_to_int(body, limit, message_index);
        body = bit_utils::bytes_to_int(body, limit, severity);
        body = bit_utils::bytes_to_int(body, limit, error_code);

        return ErrorMessage{.message_index = message_index,
                            .severity = ErrorSeverity(severity),
                            .error_code = ErrorCode(error_code)};
    }

    auto operator==(const ErrorMessage& other) const -> bool = default;
};

using HeartbeatRequest = Empty<MessageId::heartbeat_request>;

using HeartbeatResponse = Empty<MessageId::heartbeat_response>;

using MotorStatusRequest = Empty<MessageId::get_status_request>;

struct MotorStatusResponse : BaseMessage<MessageId::get_status_response> {
    uint32_t message_index;
    uint8_t enabled;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(enabled, iter, limit);
        return iter - body;
    }
    auto operator==(const MotorStatusResponse& other) const -> bool = default;
};

struct GearMotorStatusResponse
    : BaseMessage<MessageId::get_gear_status_response> {
    uint32_t message_index;
    uint8_t enabled;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(enabled, iter, limit);
        return iter - body;
    }
    auto operator==(const GearMotorStatusResponse& other) const
        -> bool = default;
};

using DeviceInfoRequest = Empty<MessageId::device_info_request>;

struct DeviceInfoResponse : BaseMessage<MessageId::device_info_response> {
    /**
     * The device info tells us some information that's useful for all nodes on
     * the canbus - e.g., nothing specific to pipettes or grippers, but what's
     * useful for everything. That includes versions, which encompass the
     * version number, the git sha, and some flags about whether or not this was
     * a CI build or a dev build; and the hardware revision, which is necessary
     * to specify what firmware this device should be updated with.
     */
    uint32_t message_index;
    uint32_t version;
    uint32_t flags;
    std::array<char, VERSION_SHORTSHA_SIZE> shortsha;
    char primary_revision;
    char secondary_revision;
    std::array<char, 2> tertiary_revision;
    uint8_t device_subidentifier;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(version, iter, limit);
        iter = bit_utils::int_to_bytes(flags, iter, limit);
        iter =
            std::copy_n(&shortsha[0],
                        std::min(limit - iter,
                                 static_cast<ptrdiff_t>(VERSION_SHORTSHA_SIZE)),
                        iter);
        iter = std::copy_n(
            &primary_revision,
            std::min(limit - iter, ptrdiff_t(sizeof(primary_revision))), iter);
        iter = std::copy_n(
            &secondary_revision,
            std::min(limit - iter, ptrdiff_t(sizeof(secondary_revision))),
            iter);
        iter = std::copy_n(
            &tertiary_revision[0],
            std::min(limit - iter, ptrdiff_t(sizeof(tertiary_revision))), iter);
        iter = bit_utils::int_to_bytes(device_subidentifier, iter, limit);
        return iter - body;
    }
    auto operator==(const DeviceInfoResponse& other) const -> bool = default;
};

using TaskInfoRequest = Empty<MessageId::task_info_request>;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct TaskInfoResponse : BaseMessage<MessageId::task_info_response> {
    uint32_t message_index;
    std::array<char, 12> name{};
    uint32_t runtime_counter;
    uint32_t stack_high_water_mark;
    uint16_t state;
    uint16_t priority;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = std::copy(name.cbegin(), name.cend(), iter);
        iter = bit_utils::int_to_bytes(runtime_counter, iter, limit);
        iter = bit_utils::int_to_bytes(stack_high_water_mark, iter, limit);
        iter = bit_utils::int_to_bytes(state, iter, limit);
        iter = bit_utils::int_to_bytes(priority, iter, limit);
        return iter - body;
    }

    auto operator==(const TaskInfoResponse& other) const -> bool = default;
};

using StopRequest = Empty<MessageId::stop_request>;

using EnableMotorRequest = Empty<MessageId::enable_motor_request>;

using GearEnableMotorRequest = Empty<MessageId::gear_enable_motor_request>;

using DisableMotorRequest = Empty<MessageId::disable_motor_request>;

using GearDisableMotorRequest = Empty<MessageId::gear_disable_motor_request>;

using ReadLimitSwitchRequest = Empty<MessageId::limit_sw_request>;

using MotorPositionRequest = Empty<MessageId::motor_position_request>;

using UpdateMotorPositionEstimationRequest =
    Empty<MessageId::update_motor_position_estimation_request>;

struct WriteToEEPromRequest : BaseMessage<MessageId::write_eeprom> {
    uint32_t message_index;
    eeprom::types::address address;
    eeprom::types::data_length data_length;
    eeprom::types::EepromData data;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteToEEPromRequest {
        eeprom::types::address address = 0;
        eeprom::types::data_length data_length = 0;
        eeprom::types::EepromData data{};
        uint32_t msg_ind = 0;
        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, address);
        body = bit_utils::bytes_to_int(body, limit, data_length);
        // Cap the length
        data_length = std::min(static_cast<size_t>(data_length), data.size());
        std::copy_n(body, data_length, data.begin());

        return WriteToEEPromRequest{.message_index = msg_ind,
                                    .address = address,
                                    .data_length = data_length,
                                    .data = data};
    }

    auto operator==(const WriteToEEPromRequest& other) const -> bool = default;
};

struct ReadFromEEPromRequest : BaseMessage<MessageId::read_eeprom_request> {
    uint32_t message_index;
    eeprom::types::address address;
    eeprom::types::data_length data_length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ReadFromEEPromRequest {
        eeprom::types::address address = 0;
        eeprom::types::data_length data_length = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, address);
        body = bit_utils::bytes_to_int(body, limit, data_length);

        return ReadFromEEPromRequest{.message_index = msg_ind,
                                     .address = address,
                                     .data_length = data_length};
    }

    auto operator==(const ReadFromEEPromRequest& other) const -> bool = default;
};

struct ReadFromEEPromResponse : BaseMessage<MessageId::read_eeprom_response> {
    uint32_t message_index;
    eeprom::types::address address;
    eeprom::types::data_length data_length;
    eeprom::types::EepromData data;
    static constexpr uint8_t SIZE = sizeof(message_index) + sizeof(address) +
                                    sizeof(data_length) +
                                    eeprom::types::max_data_length;

    /**
     * Create a response message from iterator
     * @tparam DataIter byte iterator type
     * @tparam Limit end of data
     * @param data_iter beginning of data
     * @param limit end of data
     * @return new instance
     */
    template <bit_utils::ByteIterator DataIter, typename Limit>
    static auto create(uint32_t msg_ind, eeprom::types::address address,
                       DataIter data_iter, Limit limit)
        -> ReadFromEEPromResponse {
        eeprom::types::EepromData data{};
        eeprom::types::data_length data_length = std::min(
            eeprom::types::max_data_length,
            static_cast<eeprom::types::data_length>(limit - data_iter));
        std::copy_n(data_iter, data_length, data.begin());
        return ReadFromEEPromResponse{.message_index = msg_ind,
                                      .address = address,
                                      .data_length = data_length,
                                      .data = data};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(address, iter, limit);
        iter = bit_utils::int_to_bytes(data_length, iter, limit);
        iter = std::copy_n(
            data.cbegin(),
            std::min(data_length,
                     static_cast<eeprom::types::data_length>(limit - iter)),
            iter);

        // The size is the fixed size of the message or the supplied buffer
        // size. Whichever is smaller.
        return std::min(static_cast<uint8_t>(limit - body), SIZE);
    }
    auto operator==(const ReadFromEEPromResponse& other) const
        -> bool = default;
};

struct AddLinearMoveRequest : BaseMessage<MessageId::add_move_request> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t seq_id;
    stepper_timer_ticks duration;
    um_per_tick_sq acceleration;
    mm_per_tick velocity;
    uint8_t request_stop_condition;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> AddLinearMoveRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        stepper_timer_ticks duration = 0;
        um_per_tick_sq acceleration = 0;
        mm_per_tick velocity = 0;
        uint8_t request_stop_condition = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, acceleration);
        body = bit_utils::bytes_to_int(body, limit, velocity);
        body = bit_utils::bytes_to_int(body, limit, request_stop_condition);
        return AddLinearMoveRequest{
            .message_index = msg_ind,
            .group_id = group_id,
            .seq_id = seq_id,
            .duration = duration,
            .acceleration = acceleration,
            .velocity = velocity,
            .request_stop_condition = request_stop_condition,
        };
    }

    auto operator==(const AddLinearMoveRequest& other) const -> bool = default;
};

struct HomeRequest : BaseMessage<MessageId::home_request> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t seq_id;
    stepper_timer_ticks duration;
    mm_per_tick velocity;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> HomeRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        stepper_timer_ticks duration = 0;
        mm_per_tick velocity = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, velocity);

        return HomeRequest{.message_index = msg_ind,
                           .group_id = group_id,
                           .seq_id = seq_id,
                           .duration = duration,
                           .velocity = velocity};
    }

    auto operator==(const HomeRequest& other) const -> bool = default;
};

struct GetMoveGroupRequest : BaseMessage<MessageId::get_move_group_request> {
    uint32_t message_index;
    uint8_t group_id;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetMoveGroupRequest {
        uint8_t group_id = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, group_id);
        return GetMoveGroupRequest{.message_index = msg_ind,
                                   .group_id = group_id};
    }

    auto operator==(const GetMoveGroupRequest& other) const -> bool = default;
};

struct GetMoveGroupResponse : BaseMessage<MessageId::get_move_group_response> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t num_moves;
    uint32_t total_duration;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(group_id, iter, limit);
        iter = bit_utils::int_to_bytes(num_moves, iter, limit);
        iter = bit_utils::int_to_bytes(total_duration, iter, limit);
        return iter - body;
    }
    auto operator==(const GetMoveGroupResponse& other) const -> bool = default;
};

struct ExecuteMoveGroupRequest
    : BaseMessage<MessageId::execute_move_group_request> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t start_trigger;
    uint8_t cancel_trigger;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ExecuteMoveGroupRequest {
        uint8_t group_id = 0;
        uint8_t start_trigger = 0;
        uint8_t cancel_trigger = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, start_trigger);
        body = bit_utils::bytes_to_int(body, limit, cancel_trigger);
        return ExecuteMoveGroupRequest{.message_index = msg_ind,
                                       .group_id = group_id,
                                       .start_trigger = start_trigger,
                                       .cancel_trigger = cancel_trigger};
    }

    auto operator==(const ExecuteMoveGroupRequest& other) const
        -> bool = default;
};

using ClearAllMoveGroupsRequest =
    Empty<MessageId::clear_all_move_groups_request>;

struct MoveCompleted : BaseMessage<MessageId::move_completed> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t seq_id;
    uint32_t current_position_um;
    int32_t encoder_position_um;
    uint8_t position_flags;
    uint8_t ack_id;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(group_id, iter, limit);
        iter = bit_utils::int_to_bytes(seq_id, iter, limit);
        iter = bit_utils::int_to_bytes(current_position_um, iter, limit);
        iter = bit_utils::int_to_bytes(encoder_position_um, iter, limit);
        iter = bit_utils::int_to_bytes(position_flags, iter, limit);
        iter = bit_utils::int_to_bytes(ack_id, iter, limit);
        return iter - body;
    }

    auto operator==(const MoveCompleted& other) const -> bool = default;
};

struct MotorPositionResponse : BaseMessage<MessageId::motor_position_response> {
    uint32_t message_index;
    uint32_t current_position;
    int32_t encoder_position;
    uint8_t position_flags;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(current_position, iter, limit);
        iter = bit_utils::int_to_bytes(encoder_position, iter, limit);
        iter = bit_utils::int_to_bytes(position_flags, iter, limit);
        return iter - body;
    }

    auto operator==(const MotorPositionResponse& other) const -> bool = default;
};

// This response is the exact same payload as MotorPositionResponse
struct UpdateMotorPositionEstimationResponse
    : BaseMessage<MessageId::update_motor_position_estimation_response> {
    uint32_t message_index;
    uint32_t current_position;
    int32_t encoder_position;
    uint8_t position_flags;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(current_position, iter, limit);
        iter = bit_utils::int_to_bytes(encoder_position, iter, limit);
        iter = bit_utils::int_to_bytes(position_flags, iter, limit);
        return iter - body;
    }

    auto operator==(const UpdateMotorPositionEstimationResponse& other) const
        -> bool = default;
};

struct SetMotionConstraints : BaseMessage<MessageId::set_motion_constraints> {
    uint32_t message_index;
    mm_per_tick min_velocity;
    mm_per_tick max_velocity;
    um_per_tick_sq min_acceleration;
    um_per_tick_sq max_acceleration;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetMotionConstraints {
        mm_per_tick min_velocity = 0;
        mm_per_tick max_velocity = 0;
        um_per_tick_sq min_acceleration = 0;
        um_per_tick_sq max_acceleration = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, min_velocity);
        body = bit_utils::bytes_to_int(body, limit, max_velocity);
        body = bit_utils::bytes_to_int(body, limit, min_acceleration);
        body = bit_utils::bytes_to_int(body, limit, max_acceleration);
        return SetMotionConstraints{.message_index = msg_ind,
                                    .min_velocity = min_velocity,
                                    .max_velocity = max_velocity,
                                    .min_acceleration = min_acceleration,
                                    .max_acceleration = max_acceleration};
    }

    auto operator==(const SetMotionConstraints& other) const -> bool = default;
};

using GetMotionConstraintsRequest =
    Empty<MessageId::get_motion_constraints_request>;

struct GetMotionConstraintsResponse
    : BaseMessage<MessageId::get_motion_constraints_response> {
    uint32_t message_index;
    mm_per_tick min_velocity;
    mm_per_tick max_velocity;
    um_per_tick_sq min_acceleration;
    um_per_tick_sq max_acceleration;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(min_velocity, iter, limit);
        iter = bit_utils::int_to_bytes(max_velocity, iter, limit);
        iter = bit_utils::int_to_bytes(min_acceleration, iter, limit);
        iter = bit_utils::int_to_bytes(max_acceleration, iter, limit);
        return iter - body;
    }

    auto operator==(const GetMotionConstraintsResponse& other) const
        -> bool = default;
};

struct WriteMotorDriverRegister
    : BaseMessage<MessageId::write_motor_driver_register_request> {
    uint32_t message_index;
    uint8_t reg_address;
    uint32_t data;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteMotorDriverRegister {
        uint8_t reg_address = 0;
        uint32_t data = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        body = bit_utils::bytes_to_int(body, limit, data);
        return WriteMotorDriverRegister{
            .message_index = msg_ind, .reg_address = reg_address, .data = data};
    }

    auto operator==(const WriteMotorDriverRegister& other) const
        -> bool = default;
};

struct ReadMotorDriverRegister
    : BaseMessage<MessageId::read_motor_driver_register_request> {
    uint32_t message_index;
    uint8_t reg_address;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ReadMotorDriverRegister {
        uint8_t reg_address = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        return ReadMotorDriverRegister{.message_index = msg_ind,
                                       .reg_address = reg_address};
    }

    auto operator==(const ReadMotorDriverRegister& other) const
        -> bool = default;
};

struct ReadMotorDriverRegisterResponse
    : BaseMessage<MessageId::read_motor_driver_register_response> {
    uint32_t message_index;
    uint8_t reg_address;
    uint32_t data;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(reg_address, iter, limit);
        iter = bit_utils::int_to_bytes(data, iter, limit);
        return iter - body;
    }

    auto operator==(const ReadMotorDriverRegisterResponse& other) const
        -> bool = default;
};

struct WriteMotorCurrentRequest
    : BaseMessage<MessageId::write_motor_current_request> {
    uint32_t message_index;
    uint32_t hold_current;
    uint32_t run_current;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteMotorCurrentRequest {
        uint32_t hold_current = 0;
        uint32_t run_current = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, hold_current);
        body = bit_utils::bytes_to_int(body, limit, run_current);
        return WriteMotorCurrentRequest{.message_index = msg_ind,
                                        .hold_current = hold_current,
                                        .run_current = run_current};
    }

    auto operator==(const WriteMotorCurrentRequest& other) const
        -> bool = default;
};

using ReadPresenceSensingVoltageRequest =
    Empty<MessageId::read_presence_sensing_voltage_request>;

using AttachedToolsRequest = Empty<MessageId::attached_tools_request>;

struct ReadPresenceSensingVoltageResponse
    : BaseMessage<MessageId::read_presence_sensing_voltage_response> {
    uint32_t message_index;
    uint16_t z_motor;
    uint16_t a_motor;
    uint16_t gripper;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(z_motor, iter, limit);
        iter = bit_utils::int_to_bytes(a_motor, iter, limit);
        iter = bit_utils::int_to_bytes(gripper, iter, limit);
        return iter - body;
    }

    auto operator==(const ReadPresenceSensingVoltageResponse& other) const
        -> bool = default;
};

struct PushToolsDetectedNotification
    : BaseMessage<MessageId::tools_detected_notification> {
    uint32_t message_index = 0;
    can::ids::ToolType z_motor{};
    can::ids::ToolType a_motor{};
    can::ids::ToolType gripper{};

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(z_motor), iter, limit);
        iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(a_motor), iter, limit);
        iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(gripper), iter, limit);
        return iter - body;
    }

    auto operator==(const PushToolsDetectedNotification& other) const
        -> bool = default;
};

struct ReadLimitSwitchResponse : BaseMessage<MessageId::limit_sw_response> {
    uint32_t message_index;
    uint8_t switch_status;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(switch_status, iter, limit);
        return iter - body;
    }
    auto operator==(const ReadLimitSwitchResponse& other) const
        -> bool = default;
};

using InitiateFirmwareUpdate = Empty<MessageId::fw_update_initiate>;

using FirmwareUpdateStatusRequest = Empty<MessageId::fw_update_status_request>;

struct FirmwareUpdateStatusResponse
    : BaseMessage<MessageId::fw_update_status_response> {
    uint32_t message_index;
    uint32_t flags;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(flags, iter, limit);
        return iter - body;
    }
    auto operator==(const FirmwareUpdateStatusResponse& other) const
        -> bool = default;
};

struct SendAccumulatedPressureDataRequest
    : BaseMessage<MessageId::send_accumulated_pressure_data> {
    uint32_t message_index = 0;
    uint8_t sensor_id = 0;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> SendAccumulatedPressureDataRequest {
        uint32_t msg_ind = 0;
        uint8_t sensor_id = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, sensor_id);
        return SendAccumulatedPressureDataRequest{.message_index = msg_ind,
                                                  .sensor_id = sensor_id};
    }

    auto operator==(const SendAccumulatedPressureDataRequest& other) const
        -> bool = default;
};

struct ReadFromSensorRequest : BaseMessage<MessageId::read_sensor_request> {
    uint32_t message_index = 0;
    uint8_t sensor = 0;
    uint8_t sensor_id = 0;
    uint8_t offset_reading = 0;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ReadFromSensorRequest {
        uint8_t sensor = 0;
        uint8_t sensor_id = 0;
        uint8_t offset_reading = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, sensor);
        body = bit_utils::bytes_to_int(body, limit, sensor_id);
        body = bit_utils::bytes_to_int(body, limit, offset_reading);
        return ReadFromSensorRequest{.message_index = msg_ind,
                                     .sensor = sensor,
                                     .sensor_id = sensor_id,
                                     .offset_reading = offset_reading};
    }

    auto operator==(const ReadFromSensorRequest& other) const -> bool = default;
};

struct WriteToSensorRequest : BaseMessage<MessageId::write_sensor_request> {
    uint32_t message_index;
    uint8_t sensor;
    uint8_t sensor_id;
    uint16_t data;
    uint8_t reg_address;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteToSensorRequest {
        uint8_t sensor = 0;
        uint8_t sensor_id = 0;
        uint16_t data = 0;
        uint8_t reg_address = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, sensor);
        body = bit_utils::bytes_to_int(body, limit, sensor_id);
        body = bit_utils::bytes_to_int(body, limit, data);
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        return WriteToSensorRequest{.message_index = msg_ind,
                                    .sensor = sensor,
                                    .sensor_id = sensor_id,
                                    .data = data,
                                    .reg_address = reg_address};
    }

    auto operator==(const WriteToSensorRequest& other) const -> bool = default;
};

struct BaselineSensorRequest : BaseMessage<MessageId::baseline_sensor_request> {
    uint32_t message_index = 0;
    uint8_t sensor = 0;
    uint8_t sensor_id = 0;
    uint16_t number_of_reads = 1;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> BaselineSensorRequest {
        uint8_t sensor = 0;
        uint8_t sensor_id = 0;
        uint16_t number_of_reads = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, sensor);
        body = bit_utils::bytes_to_int(body, limit, sensor_id);
        body = bit_utils::bytes_to_int(body, limit, number_of_reads);
        return BaselineSensorRequest{.message_index = msg_ind,
                                     .sensor = sensor,
                                     .sensor_id = sensor_id,
                                     .number_of_reads = number_of_reads};
    }

    auto operator==(const BaselineSensorRequest& other) const -> bool = default;
};

struct BaselineSensorResponse
    : BaseMessage<MessageId::baseline_sensor_response> {
    uint32_t message_index = 0;
    can::ids::SensorType sensor{};
    can::ids::SensorId sensor_id{};
    int32_t offset_average = 0;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), iter, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint8_t>(sensor_id), iter,
                                       limit);
        iter = bit_utils::int_to_bytes(offset_average, iter, limit);
        return iter - body;
    }
    auto operator==(const BaselineSensorResponse& other) const
        -> bool = default;
};

struct ReadFromSensorResponse : BaseMessage<MessageId::read_sensor_response> {
    uint32_t message_index = 0;
    can::ids::SensorType sensor{};
    can::ids::SensorId sensor_id{};
    int32_t sensor_data = 0;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), iter, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint8_t>(sensor_id), iter,
                                       limit);
        iter = bit_utils::int_to_bytes(sensor_data, iter, limit);
        return iter - body;
    }
    auto operator==(const ReadFromSensorResponse& other) const
        -> bool = default;
};

struct SetSensorThresholdRequest
    : BaseMessage<MessageId::set_sensor_threshold_request> {
    uint32_t message_index;
    can::ids::SensorType sensor;
    can::ids::SensorId sensor_id;
    int32_t threshold;
    can::ids::SensorThresholdMode mode;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetSensorThresholdRequest {
        uint8_t sensor = 0;
        uint8_t sensor_id = 0;
        int32_t threshold = 0;
        uint8_t mode = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, sensor);
        body = bit_utils::bytes_to_int(body, limit, sensor_id);
        body = bit_utils::bytes_to_int(body, limit, threshold);
        body = bit_utils::bytes_to_int(body, limit, mode);
        return SetSensorThresholdRequest{
            .message_index = msg_ind,
            .sensor = static_cast<can::ids::SensorType>(sensor),
            .sensor_id = static_cast<can::ids::SensorId>(sensor_id),
            .threshold = threshold,
            .mode = static_cast<can::ids::SensorThresholdMode>(mode)};
    }

    auto operator==(const SetSensorThresholdRequest& other) const
        -> bool = default;
};

struct SensorThresholdResponse
    : BaseMessage<MessageId::set_sensor_threshold_response> {
    uint32_t message_index = 0;
    can::ids::SensorType sensor{};
    can::ids::SensorId sensor_id{};
    int32_t threshold = 0;
    can::ids::SensorThresholdMode mode{};

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), iter, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint8_t>(sensor_id), iter,
                                       limit);
        iter = bit_utils::int_to_bytes(threshold, iter, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint8_t>(mode), iter, limit);
        return iter - body;
    }
    auto operator==(const SensorThresholdResponse& other) const
        -> bool = default;
};

struct SetBrushedMotorVrefRequest
    : BaseMessage<MessageId::set_brushed_motor_vref_request> {
    uint32_t message_index;
    uint32_t v_ref;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetBrushedMotorVrefRequest {
        uint32_t v_ref = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, v_ref);
        return SetBrushedMotorVrefRequest{.message_index = msg_ind,
                                          .v_ref = v_ref};
    }

    auto operator==(const SetBrushedMotorVrefRequest& other) const
        -> bool = default;
};

using BrushedMotorConfRequest = Empty<MessageId::brushed_motor_conf_request>;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct BrushedMotorConfResponse
    : BaseMessage<MessageId::brushed_motor_conf_response> {
    uint32_t message_index;
    uint32_t v_ref;
    uint32_t duty_cycle;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(v_ref, iter, limit);
        iter = bit_utils::int_to_bytes(duty_cycle, iter, limit);
        return iter - body;
    }

    auto operator==(const BrushedMotorConfResponse& other) const
        -> bool = default;
};

struct SetBrushedMotorPwmRequest
    : BaseMessage<MessageId::set_brushed_motor_pwm_request> {
    uint32_t message_index;
    uint32_t duty_cycle;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetBrushedMotorPwmRequest {
        uint32_t duty_cycle = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, duty_cycle);
        return SetBrushedMotorPwmRequest{.message_index = msg_ind,
                                         .duty_cycle = duty_cycle};
    }

    auto operator==(const SetBrushedMotorPwmRequest& other) const
        -> bool = default;
};

struct AddBrushedLinearMoveRequest
    : BaseMessage<MessageId::add_brushed_linear_move_request> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t seq_id;
    brushed_timer_ticks duration;
    uint32_t duty_cycle;
    int32_t encoder_position_um;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> AddBrushedLinearMoveRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        brushed_timer_ticks duration = 0;
        uint32_t duty_cycle = 0;
        int32_t encoder_position_um = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, duty_cycle);
        body = bit_utils::bytes_to_int(body, limit, encoder_position_um);

        return AddBrushedLinearMoveRequest{
            .message_index = msg_ind,
            .group_id = group_id,
            .seq_id = seq_id,
            .duration = duration,
            .duty_cycle = duty_cycle,
            .encoder_position_um = encoder_position_um};
    }

    auto operator==(const AddBrushedLinearMoveRequest& other) const
        -> bool = default;
};

struct GripperGripRequest : BaseMessage<MessageId::gripper_grip_request> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t seq_id;
    brushed_timer_ticks duration;
    uint32_t duty_cycle;
    int32_t encoder_position_um;
    uint8_t stay_engaged;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GripperGripRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        brushed_timer_ticks duration = 0;
        uint32_t duty_cycle = 0;
        int32_t encoder_position_um = 0;
        uint8_t stay_engaged = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, duty_cycle);
        body = bit_utils::bytes_to_int(body, limit, encoder_position_um);
        body = bit_utils::bytes_to_int(body, limit, stay_engaged);

        return GripperGripRequest{.message_index = msg_ind,
                                  .group_id = group_id,
                                  .seq_id = seq_id,
                                  .duration = duration,
                                  .duty_cycle = duty_cycle,
                                  .encoder_position_um = encoder_position_um,
                                  .stay_engaged = stay_engaged};
    }

    auto operator==(const GripperGripRequest& other) const -> bool = default;
};

struct GripperHomeRequest : BaseMessage<MessageId::gripper_home_request> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t seq_id;
    brushed_timer_ticks duration;
    uint32_t duty_cycle;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GripperHomeRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        brushed_timer_ticks duration = 0;
        uint32_t duty_cycle = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, duty_cycle);

        return GripperHomeRequest{.message_index = msg_ind,
                                  .group_id = group_id,
                                  .seq_id = seq_id,
                                  .duration = duration,
                                  .duty_cycle = duty_cycle};
    }

    auto operator==(const GripperHomeRequest& other) const -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct GripperInfoResponse : BaseMessage<MessageId::gripper_info_response> {
    uint32_t message_index;
    uint16_t model;
    eeprom::serial_number::SerialDataCodeType serial{};

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(model, iter, limit);
        iter = std::copy_n(serial.cbegin(),
                           std::min(static_cast<size_t>(serial.size()),
                                    static_cast<size_t>(limit - iter)),
                           iter);
        return iter - body;
    }

    auto operator==(const GripperInfoResponse& other) const -> bool = default;
};

struct SetSerialNumber : BaseMessage<MessageId::set_serial_number> {
    uint32_t message_index;
    eeprom::serial_number::SerialNumberType serial{};

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetSerialNumber {
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        auto return_struct = SetSerialNumber{.message_index = msg_ind};
        std::copy_n(body,
                    std::min(static_cast<std::size_t>(limit - body),
                             return_struct.serial.size()),
                    return_struct.serial.begin());
        return return_struct;
    }

    auto operator==(const SetSerialNumber& other) const -> bool = default;
};

struct SensorDiagnosticRequest
    : BaseMessage<MessageId::sensor_diagnostic_request> {
    uint32_t message_index;
    uint8_t sensor;
    uint8_t sensor_id;
    uint8_t reg_address;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SensorDiagnosticRequest {
        uint8_t sensor = 0;
        uint8_t sensor_id = 0;
        uint8_t reg_address = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, sensor);
        body = bit_utils::bytes_to_int(body, limit, sensor_id);
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        return SensorDiagnosticRequest{.message_index = msg_ind,
                                       .sensor = sensor,
                                       .sensor_id = sensor_id,
                                       .reg_address = reg_address};
    }

    auto operator==(const SensorDiagnosticRequest& other) const
        -> bool = default;
};

struct SensorDiagnosticResponse
    : BaseMessage<MessageId::sensor_diagnostic_response> {
    uint32_t message_index;
    can::ids::SensorType sensor;
    can::ids::SensorId sensor_id;
    uint8_t reg_address;
    uint32_t data;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), iter, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint8_t>(sensor_id), iter,
                                       limit);
        iter = bit_utils::int_to_bytes(reg_address, iter, limit);
        iter = bit_utils::int_to_bytes(data, iter, limit);
        return iter - body;
    }
    auto operator==(const SensorDiagnosticResponse& other) const
        -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct PipetteInfoResponse : BaseMessage<MessageId::pipette_info_response> {
    uint32_t message_index;
    uint16_t name;
    uint16_t model;
    eeprom::serial_number::SerialDataCodeType serial{};

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(name, iter, limit);
        iter = bit_utils::int_to_bytes(model, iter, limit);
        iter = std::copy_n(serial.cbegin(),
                           std::min(static_cast<size_t>(serial.size()),
                                    static_cast<size_t>(limit - iter)),
                           iter);
        return iter - body;
    }

    auto operator==(const PipetteInfoResponse& other) const -> bool = default;
};

struct BindSensorOutputRequest
    : BaseMessage<MessageId::bind_sensor_output_request> {
    uint32_t message_index;
    can::ids::SensorType sensor;
    can::ids::SensorId sensor_id;
    uint8_t binding;  // a bitfield of can::ids::SensorOutputBinding

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> BindSensorOutputRequest {
        uint8_t _sensor = 0;
        uint8_t _id = 0;
        uint8_t _binding = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, _sensor);
        body = bit_utils::bytes_to_int(body, limit, _id);
        body = bit_utils::bytes_to_int(body, limit, _binding);
        return BindSensorOutputRequest{
            .message_index = msg_ind,
            .sensor = static_cast<can::ids::SensorType>(_sensor),
            .sensor_id = static_cast<can::ids::SensorId>(_id),
            .binding = _binding};
    }

    auto operator==(const BindSensorOutputRequest& other) const
        -> bool = default;
};

struct BindSensorOutputResponse
    : BaseMessage<MessageId::bind_sensor_output_response> {
    uint32_t message_index = 0;
    can::ids::SensorType sensor{};
    can::ids::SensorId sensor_id{};
    uint8_t binding{};  // a bitfield of can::ids::SensorOutputBinding

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), iter, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint8_t>(sensor_id), iter,
                                       limit);
        iter = bit_utils::int_to_bytes(binding, iter, limit);
        return iter - body;
    }

    auto operator==(const BindSensorOutputResponse& other) const
        -> bool = default;
};

using TipStatusQueryRequest = Empty<MessageId::get_tip_status_request>;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct PushTipPresenceNotification
    : BaseMessage<MessageId::tip_presence_notification> {
    uint32_t message_index;
    uint8_t ejector_flag_status;
    can::ids::SensorId sensor_id{};

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(ejector_flag_status, iter, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint8_t>(sensor_id), iter,
                                       limit);
        return iter - body;
    }

    auto operator==(const PushTipPresenceNotification& other) const
        -> bool = default;
};

struct TipActionRequest
    : BaseMessage<MessageId::do_self_contained_tip_action_request> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t seq_id;
    stepper_timer_ticks duration;
    mm_per_tick velocity;
    can::ids::PipetteTipActionType action;
    uint8_t request_stop_condition;
    um_per_tick_sq acceleration;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> TipActionRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        stepper_timer_ticks duration = 0;
        mm_per_tick velocity = 0;
        uint8_t _action = 0;
        uint8_t request_stop_condition = 0;
        um_per_tick_sq acceleration = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, velocity);
        body = bit_utils::bytes_to_int(body, limit, _action);
        body = bit_utils::bytes_to_int(body, limit, request_stop_condition);
        body = bit_utils::bytes_to_int(body, limit, acceleration);

        return TipActionRequest{
            .message_index = msg_ind,
            .group_id = group_id,
            .seq_id = seq_id,
            .duration = duration,
            .velocity = velocity,
            .action = static_cast<can::ids::PipetteTipActionType>(_action),
            .request_stop_condition = request_stop_condition,
            .acceleration = acceleration};
    }

    auto operator==(const TipActionRequest& other) const -> bool = default;
};

struct TipActionResponse
    : BaseMessage<MessageId::do_self_contained_tip_action_response> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t seq_id;
    uint32_t current_position_um;
    int32_t encoder_position_um;
    uint8_t position_flags;
    uint8_t ack_id;
    can::ids::PipetteTipActionType action;
    uint8_t success;
    can::ids::GearMotorId gear_motor_id;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(group_id, iter, limit);
        iter = bit_utils::int_to_bytes(seq_id, iter, limit);
        iter = bit_utils::int_to_bytes(current_position_um, iter, limit);
        iter = bit_utils::int_to_bytes(encoder_position_um, iter, limit);
        iter = bit_utils::int_to_bytes(position_flags, iter, limit);
        iter = bit_utils::int_to_bytes(ack_id, iter, limit);
        iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(action), iter, limit);
        iter = bit_utils::int_to_bytes(success, iter, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint8_t>(gear_motor_id),
                                       iter, limit);
        return iter - body;
    }

    auto operator==(const TipActionResponse& other) const -> bool = default;
};

struct GearWriteMotorCurrentRequest
    : BaseMessage<MessageId::gear_set_current_request> {
    uint32_t message_index;
    uint32_t hold_current;
    uint32_t run_current;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GearWriteMotorCurrentRequest {
        uint32_t hold_current = 0;
        uint32_t run_current = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, hold_current);
        body = bit_utils::bytes_to_int(body, limit, run_current);
        return GearWriteMotorCurrentRequest{.message_index = msg_ind,
                                            .hold_current = hold_current,
                                            .run_current = run_current};
    }

    auto operator==(const GearWriteMotorCurrentRequest& other) const
        -> bool = default;
};

struct GearWriteMotorDriverRegister
    : BaseMessage<MessageId::gear_write_motor_driver_request> {
    uint32_t message_index;
    uint8_t reg_address;
    uint32_t data;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GearWriteMotorDriverRegister {
        uint8_t reg_address = 0;
        uint32_t data = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        body = bit_utils::bytes_to_int(body, limit, data);
        return GearWriteMotorDriverRegister{
            .message_index = msg_ind, .reg_address = reg_address, .data = data};
    }

    auto operator==(const GearWriteMotorDriverRegister& other) const
        -> bool = default;
};

struct GearReadMotorDriverRegister
    : BaseMessage<MessageId::gear_read_motor_driver_request> {
    uint32_t message_index;
    uint8_t reg_address;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GearReadMotorDriverRegister {
        uint8_t reg_address = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        return GearReadMotorDriverRegister{.message_index = msg_ind,
                                           .reg_address = reg_address};
    }

    auto operator==(const GearReadMotorDriverRegister& other) const
        -> bool = default;
};

struct PeripheralStatusRequest
    : BaseMessage<MessageId::peripheral_status_request> {
    uint32_t message_index;
    can::ids::SensorType sensor;
    can::ids::SensorId sensor_id;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> PeripheralStatusRequest {
        uint8_t _sensor = 0;
        uint8_t _id = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, _sensor);
        body = bit_utils::bytes_to_int(body, limit, _id);
        return PeripheralStatusRequest{
            .message_index = msg_ind,
            .sensor = static_cast<can::ids::SensorType>(_sensor),
            .sensor_id = static_cast<can::ids::SensorId>(_id)};
    }
    auto operator==(const PeripheralStatusRequest& other) const
        -> bool = default;
};

struct PeripheralStatusResponse
    : BaseMessage<MessageId::peripheral_status_response> {
    uint32_t message_index;
    can::ids::SensorType sensor;
    can::ids::SensorId sensor_id;
    uint8_t status;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), iter, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint8_t>(sensor_id), body,
                                       limit);
        iter = bit_utils::int_to_bytes(status, body, limit);
        return iter - body;
    }

    auto operator==(const PeripheralStatusResponse& other) const
        -> bool = default;
};

using InstrumentInfoRequest = Empty<MessageId::instrument_info_request>;

struct SetGripperErrorToleranceRequest
    : BaseMessage<MessageId::set_gripper_error_tolerance> {
    uint32_t message_index;
    uint32_t max_pos_error_mm;
    uint32_t max_unwanted_movement_mm;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> SetGripperErrorToleranceRequest {
        uint32_t pos_error = 0;
        uint32_t unwanted_movement = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, pos_error);
        body = bit_utils::bytes_to_int(body, limit, unwanted_movement);
        return SetGripperErrorToleranceRequest{
            .message_index = msg_ind,
            .max_pos_error_mm = pos_error,
            .max_unwanted_movement_mm = unwanted_movement};
    }
    auto operator==(const SetGripperErrorToleranceRequest& other) const
        -> bool = default;
};

using GetMotorUsageRequest = Empty<MessageId::get_motor_usage_request>;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct GetMotorUsageResponse
    : BaseMessage<MessageId::get_motor_usage_response> {
    uint32_t message_index;
    uint8_t num_keys;
    struct __attribute__((__packed__)) UsageValueField {
        uint16_t key;
        uint16_t len;
        uint64_t value;
    };
    std::array<UsageValueField, 5> values{};

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(num_keys, iter, limit);

        for (auto v : values) {
            iter = bit_utils::int_to_bytes(v.key, iter, limit);
            // dropping len to 1 byte since the max length is 8
            iter = bit_utils::int_to_bytes(uint8_t(v.len & 0xFF), iter, limit);
            iter = bit_utils::int_to_bytes(v.value, iter, limit);
        }
        return iter - body;
    }

    auto operator==(const GetMotorUsageResponse& other) const -> bool = default;
};

using GripperJawStateRequest = Empty<MessageId::gripper_jaw_state_request>;

struct GripperJawStateResponse
    : BaseMessage<MessageId::gripper_jaw_state_response> {
    uint32_t message_index;
    uint8_t jaw_state;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint8_t>(jaw_state), iter,
                                       limit);
        return iter - body;
    }

    auto operator==(const GripperJawStateResponse& other) const
        -> bool = default;
};

struct SetGripperJawHoldoffRequest
    : BaseMessage<MessageId::set_gripper_jaw_holdoff_request> {
    uint32_t message_index;
    uint32_t holdoff_ms;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetGripperJawHoldoffRequest {
        uint32_t holdoff_ms = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, holdoff_ms);
        return SetGripperJawHoldoffRequest{.message_index = msg_ind,
                                           .holdoff_ms = holdoff_ms};
    }
    auto operator==(const SetGripperJawHoldoffRequest& other) const
        -> bool = default;
};

using GripperJawHoldoffRequest = Empty<MessageId::gripper_jaw_holdoff_request>;

struct GripperJawHoldoffResponse
    : BaseMessage<MessageId::gripper_jaw_holdoff_response> {
    uint32_t message_index;
    uint32_t holdoff_ms;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(static_cast<uint32_t>(holdoff_ms), iter,
                                       limit);
        return iter - body;
    }

    auto operator==(const GripperJawHoldoffResponse& other) const
        -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct HepaUVInfoResponse : BaseMessage<MessageId::hepauv_info_response> {
    uint32_t message_index;
    uint16_t model;
    eeprom::serial_number::SerialDataCodeType serial{};

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(model, iter, limit);
        iter = std::copy_n(serial.cbegin(),
                           std::min(static_cast<size_t>(serial.size()),
                                    static_cast<size_t>(limit - iter)),
                           iter);
        return iter - body;
    }

    auto operator==(const HepaUVInfoResponse& other) const -> bool = default;
};

struct SetHepaFanStateRequest
    : BaseMessage<MessageId::set_hepa_fan_state_request> {
    uint32_t message_index;
    uint32_t duty_cycle;
    uint8_t fan_on;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetHepaFanStateRequest {
        uint8_t fan_on = 0;
        uint32_t duty_cycle = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, duty_cycle);
        body = bit_utils::bytes_to_int(body, limit, fan_on);
        return SetHepaFanStateRequest{.message_index = msg_ind,
                                      .duty_cycle = duty_cycle,
                                      .fan_on = fan_on};
    }

    auto operator==(const SetHepaFanStateRequest& other) const
        -> bool = default;
};

using GetHepaFanStateRequest = Empty<MessageId::get_hepa_fan_state_request>;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct GetHepaFanStateResponse
    : BaseMessage<MessageId::get_hepa_fan_state_response> {
    uint32_t message_index;
    uint32_t duty_cycle;
    uint8_t fan_on;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(duty_cycle, iter, limit);
        iter = bit_utils::int_to_bytes(fan_on, iter, limit);
        return iter - body;
    }

    auto operator==(const GetHepaFanStateResponse& other) const
        -> bool = default;
};

struct SetHepaUVStateRequest
    : BaseMessage<MessageId::set_hepa_uv_state_request> {
    uint32_t message_index;
    uint32_t timeout_s;
    uint8_t uv_light_on;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetHepaUVStateRequest {
        uint8_t uv_light_on = 0;
        uint32_t timeout_s = 0;
        uint32_t msg_ind = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, timeout_s);
        body = bit_utils::bytes_to_int(body, limit, uv_light_on);
        return SetHepaUVStateRequest{.message_index = msg_ind,
                                     .timeout_s = timeout_s,
                                     .uv_light_on = uv_light_on};
    }

    auto operator==(const SetHepaUVStateRequest& other) const -> bool = default;
};

using GetHepaUVStateRequest = Empty<MessageId::get_hepa_uv_state_request>;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct GetHepaUVStateResponse
    : BaseMessage<MessageId::get_hepa_uv_state_response> {
    uint32_t message_index;
    uint32_t timeout_s;
    uint8_t uv_light_on;
    uint32_t remaining_time_s;
    uint16_t uv_current_ma;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(message_index, body, limit);
        iter = bit_utils::int_to_bytes(timeout_s, iter, limit);
        iter = bit_utils::int_to_bytes(uv_light_on, iter, limit);
        iter = bit_utils::int_to_bytes(remaining_time_s, iter, limit);
        iter = bit_utils::int_to_bytes(uv_current_ma, iter, limit);
        return iter - body;
    }

    auto operator==(const GetHepaUVStateResponse& other) const
        -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct AddSensorMoveRequest : BaseMessage<MessageId::add_sensor_move_request> {
    uint32_t message_index;
    uint8_t group_id;
    uint8_t seq_id;
    stepper_timer_ticks duration;
    um_per_tick_sq acceleration;
    mm_per_tick velocity;
    uint8_t request_stop_condition;
    can::ids::SensorId sensor_id{};

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> AddSensorMoveRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        stepper_timer_ticks duration = 0;
        um_per_tick_sq acceleration = 0;
        mm_per_tick velocity = 0;
        uint8_t request_stop_condition = 0;
        uint32_t msg_ind = 0;
        uint8_t sensor_id = 0;

        body = bit_utils::bytes_to_int(body, limit, msg_ind);
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, acceleration);
        body = bit_utils::bytes_to_int(body, limit, velocity);
        body = bit_utils::bytes_to_int(body, limit, request_stop_condition);
        body = bit_utils::bytes_to_int(body, limit, sensor_id);
        return AddSensorMoveRequest{
            .message_index = msg_ind,
            .group_id = group_id,
            .seq_id = seq_id,
            .duration = duration,
            .acceleration = acceleration,
            .velocity = velocity,
            .request_stop_condition = request_stop_condition,
            .sensor_id = static_cast<can::ids::SensorId>(sensor_id),
        };
    }

    auto operator==(const AddSensorMoveRequest& other) const -> bool = default;
};

/**
 * A variant of all message types we might send..
 */

using ResponseMessageType = std::variant<
    Acknowledgment, HeartbeatResponse, ErrorMessage, DeviceInfoResponse,
    GetMotionConstraintsResponse, GetMoveGroupResponse, StopRequest,
    ReadMotorDriverRegisterResponse, ReadFromEEPromResponse, MoveCompleted,
    ReadPresenceSensingVoltageResponse, PushToolsDetectedNotification,
    ReadLimitSwitchResponse, MotorPositionResponse, ReadFromSensorResponse,
    FirmwareUpdateStatusResponse, SensorThresholdResponse,
    SensorDiagnosticResponse, TaskInfoResponse, PipetteInfoResponse,
    BindSensorOutputResponse, GripperInfoResponse, TipActionResponse,
    PeripheralStatusResponse, BrushedMotorConfResponse,
    UpdateMotorPositionEstimationResponse, BaselineSensorResponse,
    PushTipPresenceNotification, GetMotorUsageResponse, GripperJawStateResponse,
    GripperJawHoldoffResponse, HepaUVInfoResponse, GetHepaFanStateResponse,
    GetHepaUVStateResponse, MotorStatusResponse, GearMotorStatusResponse>;

}  // namespace can::messages
