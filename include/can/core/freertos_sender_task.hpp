#pragma once

#include <array>

#include "FreeRTOS.h"
#include "can/core/can_bus.hpp"
#include "can/core/message_core.hpp"
#include "common/core/logging.hpp"
#include "common/core/message_queue.hpp"

namespace freertos_sender_task {

struct TaskMessage {
    uint32_t arbitration_id;
    CanFDMessageLength data_length;
    std::array<uint8_t, message_core::MaxMessageSize> data;
};

/**
 * Entry point for a CAN sender class.
 * @tparam QueueImpl
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class MessageSenderTask {
  public:
    using QueueType = QueueImpl<TaskMessage>;

    /**
     * Constructor
     * @param can The can bus instance
     * @param queue The message queue instance.
     */
    MessageSenderTask(can_bus::CanBus& can, QueueType& queue)
        : can{can}, queue{queue} {}

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()() {
        TaskMessage message;
        while (true) {
            if (queue.try_read(&message, portMAX_DELAY)) {
                LOG("MessageSenderTask: arbid=%X length=%d\n",
                    message.arbitration_id, message.data_length);
                can.send(message.arbitration_id, message.data.begin(),
                         message.data_length);
            }
        }
    }

    auto get_queue() const -> QueueType& { return queue; }

  private:
    can_bus::CanBus& can;
    QueueType& queue;
};

}  // namespace freertos_sender_task