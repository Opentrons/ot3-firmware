#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "motor-control/core/utils.hpp"

namespace move_status_reporter_task {

using TaskMessage = motor_control_task_messages::MoveStatusReporterTaskMessage;

/**
 * The handler of move status messages
 */
template <can::message_writer_task::TaskClient CanClient,
          lms::MotorMechanicalConfig LmsConfig>
class MoveStatusMessageHandler {
  public:
    MoveStatusMessageHandler(
        CanClient& can_client,
        const lms::LinearMotionSystemConfig<LmsConfig>& lms_config)
        : can_client{can_client},
          lms_config(lms_config),
          um_per_step(
              convert_to_fixed_point_64_bit(lms_config.get_um_per_step(), 31)),
          um_per_encoder_pulse(convert_to_fixed_point_64_bit(
              lms_config.get_encoder_um_per_pulse(), 31)) {}
    MoveStatusMessageHandler(const MoveStatusMessageHandler& c) = delete;
    MoveStatusMessageHandler(const MoveStatusMessageHandler&& c) = delete;
    auto operator=(const MoveStatusMessageHandler& c) = delete;
    auto operator=(const MoveStatusMessageHandler&& c) = delete;
    ~MoveStatusMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        can::messages::MoveCompleted msg = {
            .group_id = message.group_id,
            .seq_id = message.seq_id,
            .current_position_um = fixed_point_multiply(
                um_per_step, message.current_position_steps),
            .encoder_position = fixed_point_multiply(um_per_encoder_pulse,
                                                     message.encoder_position),
            .ack_id = static_cast<uint8_t>(message.ack_id)};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

  private:
    CanClient& can_client;
    const lms::LinearMotionSystemConfig<LmsConfig>& lms_config;
    sq31_31 um_per_step;
    sq31_31 um_per_encoder_pulse;
};

template <can::message_writer_task::TaskClient CanClient>
class BrushedMoveStatusMessageHandler {
  public:
    BrushedMoveStatusMessageHandler(CanClient& can_client)
        : can_client{can_client} {}
    BrushedMoveStatusMessageHandler(const BrushedMoveStatusMessageHandler& c) =
        delete;
    BrushedMoveStatusMessageHandler(const BrushedMoveStatusMessageHandler&& c) =
        delete;
    auto operator=(const BrushedMoveStatusMessageHandler& c) = delete;
    auto operator=(const BrushedMoveStatusMessageHandler&& c) = delete;
    ~BrushedMoveStatusMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        can::messages::MoveCompleted msg = {
            .group_id = message.group_id,
            .seq_id = message.seq_id,
            .current_position_um = 0,
            .encoder_position = message.encoder_position,
            .ack_id = static_cast<uint8_t>(message.ack_id)};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

  private:
    CanClient& can_client;
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
              lms::MotorMechanicalConfig LmsConfig>
    [[noreturn]] void operator()(
        CanClient* can_client,
        const lms::LinearMotionSystemConfig<LmsConfig>* config) {
        auto handler = MoveStatusMessageHandler{*can_client, *config};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    /**
     * Brushed task entry point.
     */
    template <can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(CanClient* can_client) {
        auto handler = BrushedMoveStatusMessageHandler{*can_client};
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
