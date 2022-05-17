#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/utils.hpp"

#include "pipettes/core/tasks/messages.hpp"

namespace pipettes {

namespace tasks {

namespace gear_move_status {

using TaskMessage = pipettes::motor_control_task_messages::MoveStatusReporterTaskMessage;

template <message_writer_task::TaskClient CanClient,
        lms::MotorMechanicalConfig LmsConfig>
class MoveStatusMessageHandler {
  public:
    MoveStatusMessageHandler(
        CanClient& can_client,
        const lms::LinearMotionSystemConfig<LmsConfig>& lms_config)
        : can_client{can_client},
          lms_config(lms_config),
          um_per_step(convert_to_fixed_point_64_bit(
              lms_config.get_um_per_step(), 31)) {}
    MoveStatusMessageHandler(const MoveStatusMessageHandler& c) = delete;
    MoveStatusMessageHandler(const MoveStatusMessageHandler&& c) = delete;
    auto operator=(const MoveStatusMessageHandler& c) = delete;
    auto operator=(const MoveStatusMessageHandler&& c) = delete;
    ~MoveStatusMessageHandler() = default;

    /**
    * Handle Ack message
    */
    void handle_message(const TaskMessage& message) {
        can_messages::TipActionResponse msg = {
            .group_id = message.group_id,
            .seq_id = message.seq_id,
            .encoder_position = message.encoder_position,
            .ack_id = static_cast<uint8_t>(message.ack_id),
            // How to get tip info into here
            .success = true};
        can_client.send_can_message(can_ids::NodeId::host, msg);
    }

  private:
    CanClient& can_client;
    const lms::LinearMotionSystemConfig<LmsConfig>& lms_config;
    sq31_31 um_per_step;
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
    template <message_writer_task::TaskClient CanClient,
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

        } // namespace gear_move_status
    } // namespace tasks
} // namespace pipettes

