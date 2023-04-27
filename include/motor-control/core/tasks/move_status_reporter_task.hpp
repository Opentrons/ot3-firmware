#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"
#include "motor-control/core/utils.hpp"

namespace move_status_reporter_task {

using TaskMessage = motor_control_task_messages::MoveStatusReporterTaskMessage;

/**
 * The handler of move status messages
 */
template <can::message_writer_task::TaskClient CanClient,
          lms::MotorMechanicalConfig LmsConfig,
          usage_storage_task::TaskClient UsageClient>
class MoveStatusMessageHandler {
  public:
    MoveStatusMessageHandler(
        CanClient& can_client,
        const lms::LinearMotionSystemConfig<LmsConfig>& lms_config,
        UsageClient& usage_client)
        : can_client{can_client},
          lms_config(lms_config),
          um_per_step(
              convert_to_fixed_point_64_bit(lms_config.get_um_per_step(), 31)),
          um_per_encoder_pulse(convert_to_fixed_point_64_bit(
              lms_config.get_encoder_um_per_pulse(), 31)),
          usage_client(usage_client) {}
    MoveStatusMessageHandler(const MoveStatusMessageHandler& c) = delete;
    MoveStatusMessageHandler(const MoveStatusMessageHandler&& c) = delete;
    auto operator=(const MoveStatusMessageHandler& c) = delete;
    auto operator=(const MoveStatusMessageHandler&& c) = delete;
    ~MoveStatusMessageHandler() = default;

    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle_message(m); }, message);
    }

    void handle_message(std::monostate&) {}

    void handle_message(const can::messages::ErrorMessage& msg) {
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    void handle_message(const can::messages::StopRequest& msg) {
        can_client.send_can_message(can::ids::NodeId::broadcast, msg);
    }

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const motor_messages::Ack& message) {
        int32_t end_position = fixed_point_multiply(
            um_per_encoder_pulse, message.encoder_position, radix_offset_0{});
        can::messages::MoveCompleted msg = {
            .message_index = message.message_index,
            .group_id = message.group_id,
            .seq_id = message.seq_id,
            .current_position_um = fixed_point_multiply(
                um_per_step, message.current_position_steps),
            .encoder_position_um = end_position,
            .position_flags = message.position_flags,
            .ack_id = static_cast<uint8_t>(message.ack_id)};
        can_client.send_can_message(can::ids::NodeId::host, msg);

        int32_t distance_traveled_um =
            end_position - fixed_point_multiply(um_per_encoder_pulse,
                                                message.start_encoder_position,
                                                radix_offset_0{});
        usage_client.send_usage_storage_queue(
            usage_messages::IncreaseDistanceUsage{
                .key = message.usage_key,
                .distance_traveled_um =
                    uint32_t(std::abs(distance_traveled_um))});
    }

    void handle_message(const motor_messages::UpdatePositionResponse& message) {
        can::messages::UpdateMotorPositionEstimationResponse msg = {
            .message_index = message.message_index,
            .current_position = fixed_point_multiply(
                um_per_step, message.stepper_position_counts),
            .encoder_position = fixed_point_multiply(
                um_per_encoder_pulse, message.encoder_pulses, radix_offset_0{}),
            .position_flags = message.position_flags};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    void handle_message(const usage_messages::IncreaseForceTimeUsage& message) {
        usage_client.send_usage_storage_queue(message);
    }

  private:
    CanClient& can_client;
    const lms::LinearMotionSystemConfig<LmsConfig>& lms_config;
    sq31_31 um_per_step;
    sq31_31 um_per_encoder_pulse;
    UsageClient& usage_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class MoveStatusReporterTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    MoveStatusReporterTask(QueueType& queue) : queue{queue} {}
    MoveStatusReporterTask(const MoveStatusReporterTask& c) = delete;
    MoveStatusReporterTask(const MoveStatusReporterTask&& c) = delete;
    auto operator=(const MoveStatusReporterTask& c) = delete;
    auto operator=(const MoveStatusReporterTask&& c) = delete;
    ~MoveStatusReporterTask() = default;

    /**
     * Task entry point.
     */
    template <can::message_writer_task::TaskClient CanClient,
              lms::MotorMechanicalConfig LmsConfig,
              usage_storage_task::TaskClient UsageClient>
    [[noreturn]] void operator()(
        CanClient* can_client,
        const lms::LinearMotionSystemConfig<LmsConfig>* config,
        UsageClient* usage_client) {
        auto handler =
            MoveStatusMessageHandler{*can_client, *config, *usage_client};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_move_status_reporter_queue(m)};
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept BrushedTaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_brushed_move_status_reporter_queue(m)};
};

}  // namespace move_status_reporter_task
