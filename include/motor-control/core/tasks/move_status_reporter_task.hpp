#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue_poller.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace move_status_reporter_task {

/**
 * The message type this task accepts.
 */
using TaskMessage = motor_messages::Ack;


/**
 * The handler of move status messages
 */
class MoveStatusMessageHandler {
  public:
    MoveStatusMessageHandler() = default;
    ~MoveStatusMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        can_messages::MoveCompleted msg = {
            .group_id = message.group_id,
            .seq_id = message.seq_id,
            .current_position = message.current_position,
            .ack_id = static_cast<uint8_t>(
                motor_messages::AckMessageId::complete),
        };
        //        message_writer.write(NodeId::host, msg);
    }

  private:
};

/**
 * The task type.
 */
using MoveStatusReporterTask =
    freertos_message_queue_poller::FreeRTOSMessageQueuePoller<
        TaskMessage, MoveStatusMessageHandler>;

}  // namespace motor_driver_task