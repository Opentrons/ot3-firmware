#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/version.h"
#include "eeprom//core/types.hpp"
#include "parse.hpp"

namespace can_messages {

using namespace can_ids;

using stepper_timer_ticks = uint32_t;
using brushed_timer_ticks = uint32_t;
using um_per_tick = int32_t;
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
    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input, Limit) -> Empty {
        return Empty{};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output, Limit) const -> uint8_t {
        return 0;
    }

    auto operator==(const Empty&) const -> bool = default;
};

using HeartbeatRequest = Empty<MessageId::heartbeat_request>;

using HeartbeatResponse = Empty<MessageId::heartbeat_response>;

using DeviceInfoRequest = Empty<MessageId::device_info_request>;

struct DeviceInfoResponse : BaseMessage<MessageId::device_info_response> {
    /**
     *   TODO (al, 2021-09-13)
     *   Seth's thoughts on future of payload
     *   IMO we should set this up for a couple more things than just version,
     * and then can care about version in general a little less. I don't think
     * it's necessarily critical to get right the first time, but we could do
     * something like:
     *   - two bits of build type (dev, testing, release)
     *   - a byte or two of like message schema version
     *   - a four byte incrementing version seems fine i suppose
     *   - a byte or two for well-known-id system identification through some
     * centrally defined enum
     *   - at some point we'll want a serial number probably
     *   - a hardware revision unless we want to fold that into the
     * well-known-id
     */
    uint32_t version;
    uint32_t flags;
    std::array<char, VERSION_SHORTSHA_SIZE> shortsha;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(version, body, limit);
        iter = bit_utils::int_to_bytes(flags, iter, limit);
        iter =
            std::copy_n(&shortsha[0],
                        std::min(limit - iter,
                                 static_cast<ptrdiff_t>(VERSION_SHORTSHA_SIZE)),
                        iter);

        return iter - body;
    }
    auto operator==(const DeviceInfoResponse& other) const -> bool = default;
};

using TaskInfoRequest = Empty<MessageId::task_info_request>;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct TaskInfoResponse : BaseMessage<MessageId::task_info_response> {
    std::array<char, 12> name{};
    uint32_t runtime_counter;
    uint32_t stack_high_water_mark;
    uint16_t state;
    uint16_t priority;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = std::copy(name.cbegin(), name.cend(), body);
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

using DisableMotorRequest = Empty<MessageId::disable_motor_request>;

using SetupRequest = Empty<MessageId::setup_request>;

using ReadLimitSwitchRequest = Empty<MessageId::limit_sw_request>;

struct WriteToEEPromRequest : BaseMessage<MessageId::write_eeprom> {
    eeprom::types::address address;
    eeprom::types::data_length data_length;
    eeprom::types::EepromData data;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteToEEPromRequest {
        eeprom::types::address address = 0;
        eeprom::types::data_length data_length = 0;
        eeprom::types::EepromData data{};

        body = bit_utils::bytes_to_int(body, limit, address);
        body = bit_utils::bytes_to_int(body, limit, data_length);
        // Cap the length
        data_length = std::min(static_cast<size_t>(data_length), data.size());
        std::copy_n(body, data_length, data.begin());

        return WriteToEEPromRequest{
            .address = address, .data_length = data_length, .data = data};
    }

    auto operator==(const WriteToEEPromRequest& other) const -> bool = default;
};

struct ReadFromEEPromRequest : BaseMessage<MessageId::read_eeprom_request> {
    eeprom::types::address address;
    eeprom::types::data_length data_length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ReadFromEEPromRequest {
        eeprom::types::address address = 0;
        eeprom::types::data_length data_length = 0;

        body = bit_utils::bytes_to_int(body, limit, address);
        body = bit_utils::bytes_to_int(body, limit, data_length);

        return ReadFromEEPromRequest{.address = address,
                                     .data_length = data_length};
    }

    auto operator==(const ReadFromEEPromRequest& other) const -> bool = default;
};

struct ReadFromEEPromResponse : BaseMessage<MessageId::read_eeprom_response> {
    eeprom::types::address address;
    eeprom::types::data_length data_length;
    eeprom::types::EepromData data;

    /**
     * Create a response message from iterator
     * @tparam DataIter byte iterator type
     * @tparam Limit end of data
     * @param data_iter beginning of data
     * @param limit end of data
     * @return new instance
     */
    template <bit_utils::ByteIterator DataIter, typename Limit>
    static auto create(eeprom::types::address address, DataIter data_iter,
                       Limit limit) -> ReadFromEEPromResponse {
        eeprom::types::EepromData data{};
        eeprom::types::data_length data_length = std::min(
            eeprom::types::max_data_length,
            static_cast<eeprom::types::data_length>(limit - data_iter));
        std::copy_n(data_iter, data_length, data.begin());
        return ReadFromEEPromResponse{
            .address = address, .data_length = data_length, .data = data};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(address, body, limit);
        iter = bit_utils::int_to_bytes(data_length, iter, limit);
        iter = std::copy_n(
            data.cbegin(),
            std::min(data_length,
                     static_cast<eeprom::types::data_length>(limit - iter)),
            iter);
        return iter - body;
    }
    auto operator==(const ReadFromEEPromResponse& other) const
        -> bool = default;
};

struct AddLinearMoveRequest : BaseMessage<MessageId::add_move_request> {
    uint8_t group_id;
    uint8_t seq_id;
    stepper_timer_ticks duration;
    um_per_tick_sq acceleration;
    um_per_tick velocity;
    uint8_t request_stop_condition;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> AddLinearMoveRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        stepper_timer_ticks duration = 0;
        um_per_tick_sq acceleration = 0;
        um_per_tick velocity = 0;
        uint8_t request_stop_condition = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, acceleration);
        body = bit_utils::bytes_to_int(body, limit, velocity);
        body = bit_utils::bytes_to_int(body, limit, request_stop_condition);
        return AddLinearMoveRequest{
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
    uint8_t group_id;
    uint8_t seq_id;
    stepper_timer_ticks duration;
    um_per_tick velocity;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> HomeRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        stepper_timer_ticks duration = 0;
        um_per_tick velocity = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, velocity);

        return HomeRequest{.group_id = group_id,
                           .seq_id = seq_id,
                           .duration = duration,
                           .velocity = velocity};
    }

    auto operator==(const HomeRequest& other) const -> bool = default;
};

struct GetMoveGroupRequest : BaseMessage<MessageId::get_move_group_request> {
    uint8_t group_id;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GetMoveGroupRequest {
        uint8_t group_id = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        return GetMoveGroupRequest{.group_id = group_id};
    }

    auto operator==(const GetMoveGroupRequest& other) const -> bool = default;
};

struct GetMoveGroupResponse : BaseMessage<MessageId::get_move_group_response> {
    uint8_t group_id;
    uint8_t num_moves;
    uint32_t total_duration;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(group_id, body, limit);
        iter = bit_utils::int_to_bytes(num_moves, iter, limit);
        iter = bit_utils::int_to_bytes(total_duration, iter, limit);
        return iter - body;
    }
    auto operator==(const GetMoveGroupResponse& other) const -> bool = default;
};

struct ExecuteMoveGroupRequest
    : BaseMessage<MessageId::execute_move_group_request> {
    uint8_t group_id;
    uint8_t start_trigger;
    uint8_t cancel_trigger;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ExecuteMoveGroupRequest {
        uint8_t group_id = 0;
        uint8_t start_trigger = 0;
        uint8_t cancel_trigger = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, start_trigger);
        body = bit_utils::bytes_to_int(body, limit, cancel_trigger);
        return ExecuteMoveGroupRequest{.group_id = group_id,
                                       .start_trigger = start_trigger,
                                       .cancel_trigger = cancel_trigger};
    }

    auto operator==(const ExecuteMoveGroupRequest& other) const
        -> bool = default;
};

using ClearAllMoveGroupsRequest =
    Empty<MessageId::clear_all_move_groups_request>;

struct MoveCompleted : BaseMessage<MessageId::move_completed> {
    uint8_t group_id;
    uint8_t seq_id;
    uint32_t current_position_um;
    uint32_t encoder_position;
    uint8_t ack_id;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(group_id, body, limit);
        iter = bit_utils::int_to_bytes(seq_id, iter, limit);
        iter = bit_utils::int_to_bytes(current_position_um, iter, limit);
        iter = bit_utils::int_to_bytes(encoder_position, iter, limit);
        iter = bit_utils::int_to_bytes(ack_id, iter, limit);
        return iter - body;
    }

    auto operator==(const MoveCompleted& other) const -> bool = default;
};

struct SetMotionConstraints : BaseMessage<MessageId::set_motion_constraints> {
    um_per_tick min_velocity;
    um_per_tick max_velocity;
    um_per_tick_sq min_acceleration;
    um_per_tick_sq max_acceleration;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetMotionConstraints {
        um_per_tick min_velocity = 0;
        um_per_tick max_velocity = 0;
        um_per_tick_sq min_acceleration = 0;
        um_per_tick_sq max_acceleration = 0;
        body = bit_utils::bytes_to_int(body, limit, min_velocity);
        body = bit_utils::bytes_to_int(body, limit, max_velocity);
        body = bit_utils::bytes_to_int(body, limit, min_acceleration);
        body = bit_utils::bytes_to_int(body, limit, max_acceleration);
        return SetMotionConstraints{.min_velocity = min_velocity,
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
    um_per_tick min_velocity;
    um_per_tick max_velocity;
    um_per_tick_sq min_acceleration;
    um_per_tick_sq max_acceleration;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(min_velocity, body, limit);
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
    uint8_t reg_address;
    uint32_t data;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteMotorDriverRegister {
        uint8_t reg_address = 0;
        uint32_t data = 0;
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        body = bit_utils::bytes_to_int(body, limit, data);
        return WriteMotorDriverRegister{.reg_address = reg_address,
                                        .data = data};
    }

    auto operator==(const WriteMotorDriverRegister& other) const
        -> bool = default;
};

struct ReadMotorDriverRegister
    : BaseMessage<MessageId::read_motor_driver_register_request> {
    uint8_t reg_address;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ReadMotorDriverRegister {
        uint8_t reg_address = 0;
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        return ReadMotorDriverRegister{.reg_address = reg_address};
    }

    auto operator==(const ReadMotorDriverRegister& other) const
        -> bool = default;
};

struct ReadMotorDriverRegisterResponse
    : BaseMessage<MessageId::read_motor_driver_register_response> {
    uint8_t reg_address;
    uint32_t data;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(reg_address, body, limit);
        iter = bit_utils::int_to_bytes(data, iter, limit);
        return iter - body;
    }

    auto operator==(const ReadMotorDriverRegisterResponse& other) const
        -> bool = default;
};

struct WriteMotorCurrentRequest
    : BaseMessage<MessageId::write_motor_current_request> {
    uint32_t hold_current;
    uint32_t run_current;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteMotorCurrentRequest {
        uint32_t hold_current = 0;
        uint32_t run_current = 0;
        body = bit_utils::bytes_to_int(body, limit, hold_current);
        body = bit_utils::bytes_to_int(body, limit, run_current);
        return WriteMotorCurrentRequest{.hold_current = hold_current,
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
    uint16_t z_motor;
    uint16_t a_motor;
    uint16_t gripper;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(z_motor, body, limit);
        iter = bit_utils::int_to_bytes(a_motor, iter, limit);
        iter = bit_utils::int_to_bytes(gripper, iter, limit);
        return iter - body;
    }

    auto operator==(const ReadPresenceSensingVoltageResponse& other) const
        -> bool = default;
};

struct PushToolsDetectedNotification
    : BaseMessage<MessageId::tools_detected_notification> {
    can_ids::ToolType z_motor{};
    can_ids::ToolType a_motor{};
    can_ids::ToolType gripper{};

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(z_motor), body, limit);
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
    uint8_t switch_status;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(switch_status, body, limit);
        return iter - body;
    }
    auto operator==(const ReadLimitSwitchResponse& other) const
        -> bool = default;
};

using InitiateFirmwareUpdate = Empty<MessageId::fw_update_initiate>;

using FirmwareUpdateStatusRequest = Empty<MessageId::fw_update_status_request>;

struct FirmwareUpdateStatusResponse
    : BaseMessage<MessageId::fw_update_status_response> {
    uint32_t flags;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(flags, body, limit);
        return iter - body;
    }
    auto operator==(const FirmwareUpdateStatusResponse& other) const
        -> bool = default;
};

struct ReadFromSensorRequest : BaseMessage<MessageId::read_sensor_request> {
    uint8_t sensor = 0;
    uint8_t offset_reading = 0;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> ReadFromSensorRequest {
        uint8_t sensor = 0;
        uint8_t offset_reading = 0;
        body = bit_utils::bytes_to_int(body, limit, sensor);
        body = bit_utils::bytes_to_int(body, limit, offset_reading);
        return ReadFromSensorRequest{.sensor = sensor,
                                     .offset_reading = offset_reading};
    }

    auto operator==(const ReadFromSensorRequest& other) const -> bool = default;
};

struct WriteToSensorRequest : BaseMessage<MessageId::write_sensor_request> {
    uint8_t sensor;
    uint16_t data;
    uint8_t reg_address;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> WriteToSensorRequest {
        uint8_t sensor = 0;
        uint16_t data = 0;
        uint8_t reg_address = 0;
        body = bit_utils::bytes_to_int(body, limit, sensor);
        body = bit_utils::bytes_to_int(body, limit, data);
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        return WriteToSensorRequest{
            .sensor = sensor, .data = data, .reg_address = reg_address};
    }

    auto operator==(const WriteToSensorRequest& other) const -> bool = default;
};

struct BaselineSensorRequest : BaseMessage<MessageId::baseline_sensor_request> {
    uint8_t sensor = 0;
    uint8_t sample_rate = 1;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> BaselineSensorRequest {
        uint8_t sensor = 0;
        uint8_t sample_rate = 0;
        body = bit_utils::bytes_to_int(body, limit, sensor);
        body = bit_utils::bytes_to_int(body, limit, sample_rate);
        return BaselineSensorRequest{.sensor = sensor,
                                     .sample_rate = sample_rate};
    }

    auto operator==(const BaselineSensorRequest& other) const -> bool = default;
};

struct ReadFromSensorResponse : BaseMessage<MessageId::read_sensor_response> {
    can_ids::SensorType sensor{};
    int32_t sensor_data = 0;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), body, limit);
        iter = bit_utils::int_to_bytes(sensor_data, iter, limit);
        return iter - body;
    }
    auto operator==(const ReadFromSensorResponse& other) const
        -> bool = default;
};

struct SetSensorThresholdRequest
    : BaseMessage<MessageId::set_sensor_threshold_request> {
    uint8_t sensor;
    int32_t threshold;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetSensorThresholdRequest {
        uint8_t sensor = 0;
        int32_t threshold = 0;
        body = bit_utils::bytes_to_int(body, limit, sensor);
        body = bit_utils::bytes_to_int(body, limit, threshold);
        return SetSensorThresholdRequest{.sensor = sensor,
                                         .threshold = threshold};
    }

    auto operator==(const SetSensorThresholdRequest& other) const
        -> bool = default;
};

struct SensorThresholdResponse
    : BaseMessage<MessageId::set_sensor_threshold_response> {
    can_ids::SensorType sensor{};
    int32_t threshold = 0;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), body, limit);
        iter = bit_utils::int_to_bytes(threshold, iter, limit);
        return iter - body;
    }
    auto operator==(const SensorThresholdResponse& other) const
        -> bool = default;
};

struct SetBrushedMotorVrefRequest
    : BaseMessage<MessageId::set_brushed_motor_vref_request> {
    uint32_t v_ref;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetBrushedMotorVrefRequest {
        uint32_t v_ref = 0;
        body = bit_utils::bytes_to_int(body, limit, v_ref);
        return SetBrushedMotorVrefRequest{.v_ref = v_ref};
    }

    auto operator==(const SetBrushedMotorVrefRequest& other) const
        -> bool = default;
};

struct SetBrushedMotorPwmRequest
    : BaseMessage<MessageId::set_brushed_motor_pwm_request> {
    uint32_t freq;
    uint32_t duty_cycle;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SetBrushedMotorPwmRequest {
        uint32_t freq = 0;
        uint32_t duty_cycle = 0;
        body = bit_utils::bytes_to_int(body, limit, freq);
        body = bit_utils::bytes_to_int(body, limit, duty_cycle);
        return SetBrushedMotorPwmRequest{.freq = freq,
                                         .duty_cycle = duty_cycle};
    }

    auto operator==(const SetBrushedMotorPwmRequest& other) const
        -> bool = default;
};

struct GripperGripRequest : BaseMessage<MessageId::gripper_grip_request> {
    uint8_t group_id;
    uint8_t seq_id;
    brushed_timer_ticks duration;
    uint32_t freq;
    uint32_t duty_cycle;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GripperGripRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        brushed_timer_ticks duration = 0;
        uint32_t freq = 0;
        uint32_t duty_cycle = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, freq);
        body = bit_utils::bytes_to_int(body, limit, duty_cycle);

        return GripperGripRequest{.group_id = group_id,
                                  .seq_id = seq_id,
                                  .duration = duration,
                                  .freq = freq,
                                  .duty_cycle = duty_cycle};
    }

    auto operator==(const GripperGripRequest& other) const -> bool = default;
};

struct GripperHomeRequest : BaseMessage<MessageId::gripper_home_request> {
    uint8_t group_id;
    uint8_t seq_id;
    brushed_timer_ticks duration;
    uint32_t freq;
    uint32_t duty_cycle;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> GripperHomeRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        brushed_timer_ticks duration = 0;
        uint32_t freq = 0;
        uint32_t duty_cycle = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, freq);
        body = bit_utils::bytes_to_int(body, limit, duty_cycle);

        return GripperHomeRequest{.group_id = group_id,
                                  .seq_id = seq_id,
                                  .duration = duration,
                                  .freq = freq,
                                  .duty_cycle = duty_cycle};
    }

    auto operator==(const GripperHomeRequest& other) const -> bool = default;
};

using GripperInfoRequest = Empty<MessageId::gripper_info_request>;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct GripperInfoResponse : BaseMessage<MessageId::gripper_info_response> {
    uint16_t model;
    std::array<char, 12> serial{};

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(model, body, limit);
        iter = std::copy_n(serial.cbegin(),
                           std::min(static_cast<size_t>(serial.size()),
                                    static_cast<size_t>(limit - iter)),
                           iter);
        return iter - body;
    }

    auto operator==(const GripperInfoResponse& other) const -> bool = default;
};

struct SensorDiagnosticRequest
    : BaseMessage<MessageId::sensor_diagnostic_request> {
    uint8_t sensor;
    uint8_t reg_address;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> SensorDiagnosticRequest {
        uint8_t sensor = 0;
        uint8_t reg_address = 0;
        body = bit_utils::bytes_to_int(body, limit, sensor);
        body = bit_utils::bytes_to_int(body, limit, reg_address);
        return SensorDiagnosticRequest{.sensor = sensor,
                                       .reg_address = reg_address};
    }

    auto operator==(const SensorDiagnosticRequest& other) const
        -> bool = default;
};

struct SensorDiagnosticResponse
    : BaseMessage<MessageId::sensor_diagnostic_response> {
    can_ids::SensorType sensor;
    uint8_t reg_address;
    uint32_t data;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), body, limit);
        iter = bit_utils::int_to_bytes(reg_address, iter, limit);
        iter = bit_utils::int_to_bytes(data, iter, limit);
        return iter - body;
    }
    auto operator==(const SensorDiagnosticResponse& other) const
        -> bool = default;
};

using PipetteInfoRequest = Empty<MessageId::pipette_info_request>;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct PipetteInfoResponse : BaseMessage<MessageId::pipette_info_response> {
    uint16_t name;
    uint16_t model;
    std::array<char, 12> serial{};

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(name, body, limit);
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
    can_ids::SensorType sensor;
    uint8_t binding;  // a bitfield of can_ids::SensorOutputBinding

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> BindSensorOutputRequest {
        uint8_t _sensor = 0;
        uint8_t _binding = 0;
        body = bit_utils::bytes_to_int(body, limit, _sensor);
        body = bit_utils::bytes_to_int(body, limit, _binding);
        return BindSensorOutputRequest{
            .sensor = static_cast<can_ids::SensorType>(_sensor),
            .binding = _binding};
    }

    auto operator==(const BindSensorOutputRequest& other) const
        -> bool = default;
};

struct BindSensorOutputResponse
    : BaseMessage<MessageId::bind_sensor_output_response> {
    can_ids::SensorType sensor{};
    uint8_t binding{};  // a bitfield of can_ids::SensorOutputBinding

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), body, limit);
        iter = bit_utils::int_to_bytes(binding, iter, limit);
        return iter - body;
    }

    auto operator==(const BindSensorOutputResponse& other) const
        -> bool = default;
};

struct TipActionRequest
    : BaseMessage<MessageId::do_self_contained_tip_action_request> {
    uint8_t group_id;
    uint8_t seq_id;
    stepper_timer_ticks duration;
    um_per_tick velocity;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> TipActionRequest {
        uint8_t group_id = 0;
        uint8_t seq_id = 0;
        stepper_timer_ticks duration = 0;
        um_per_tick velocity = 0;
        body = bit_utils::bytes_to_int(body, limit, group_id);
        body = bit_utils::bytes_to_int(body, limit, seq_id);
        body = bit_utils::bytes_to_int(body, limit, duration);
        body = bit_utils::bytes_to_int(body, limit, velocity);

        return TipActionRequest{.group_id = group_id,
                                .seq_id = seq_id,
                                .duration = duration,
                                .velocity = velocity};
    }

    auto operator==(const TipActionRequest& other) const -> bool = default;
};

struct TipActionResponse
    : BaseMessage<MessageId::do_self_contained_tip_action_response> {
    uint8_t group_id;
    uint8_t seq_id;
    uint32_t current_position_um;
    uint32_t encoder_position;
    uint8_t ack_id;
    uint8_t success;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter = bit_utils::int_to_bytes(group_id, body, limit);
        iter = bit_utils::int_to_bytes(seq_id, iter, limit);
        iter = bit_utils::int_to_bytes(current_position_um, iter, limit);
        iter = bit_utils::int_to_bytes(encoder_position, iter, limit);
        iter = bit_utils::int_to_bytes(ack_id, iter, limit);
        iter = bit_utils::int_to_bytes(success, iter, limit);
        return iter - body;
    }

    auto operator==(const TipActionResponse& other) const -> bool = default;
};

struct PeripheralInfoRequest : BaseMessage<MessageId::peripheral_info_request> {
    can_ids::SensorType sensor;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> PeripheralInfoRequest {
        uint8_t _sensor = 0;
        body = bit_utils::bytes_to_int(body, limit, _sensor);
        return PeripheralInfoRequest{
            .sensor = static_cast<can_ids::SensorType>(_sensor)};
    }
    auto operator==(const PeripheralInfoRequest& other) const -> bool = default;
};

struct PeripheralInfoResponse
    : BaseMessage<MessageId::peripheral_info_response> {
    can_ids::SensorType sensor;
    uint8_t status;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> uint8_t {
        auto iter =
            bit_utils::int_to_bytes(static_cast<uint8_t>(sensor), body, limit);
        iter = bit_utils::int_to_bytes(status, body, limit);
        return iter - body;
    }

    auto operator==(const PeripheralInfoResponse& other) const
        -> bool = default;
};

/**
 * A variant of all message types we might send..
 */

using ResponseMessageType = std::variant<
    HeartbeatResponse, DeviceInfoResponse, GetMotionConstraintsResponse,
    GetMoveGroupResponse, ReadMotorDriverRegisterResponse,
    ReadFromEEPromResponse, MoveCompleted, ReadPresenceSensingVoltageResponse,
    PushToolsDetectedNotification, ReadLimitSwitchResponse,
    ReadFromSensorResponse, FirmwareUpdateStatusResponse,
    SensorThresholdResponse, SensorDiagnosticResponse, TaskInfoResponse,
    PipetteInfoResponse, BindSensorOutputResponse, GripperInfoResponse,
    TipActionResponse, PeripheralInfoResponse>;

}  // namespace can_messages
