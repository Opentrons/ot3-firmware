#pragma once

#include "freertos_message_queue.hpp"

namespace freertos_message_queue_poller {

/**
 * Concept describing a handler of messages.
 *
 * @tparam Handler Handler type
 * @tparam MessageType Message type
 */
template <class Handler, typename MessageType>
concept MessageHandler = requires(Handler handler, const MessageType& message) {
    {handler.handle_message(message)};
};

/**
 * A FreeRTOSTask entry point that polls a MessageQueue and notifies a Handler
 * with new messages.
 *
 * @tparam Message Message type
 * @tparam Handler Handler Type
 */
template <typename Message, typename Handler>
requires MessageHandler<Handler, Message>
class FreeRTOSMessageQueuePoller {
  public:
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<Message>;
    FreeRTOSMessageQueuePoller(QueueType& queue, Handler& handler)
        : queue{queue}, handler{handler} {}
    ~FreeRTOSMessageQueuePoller() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()() {
        Message message{};
        for (;;) {
            if (queue.try_read(&message, portMAX_DELAY)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
    Handler& handler;
};

}  // namespace freertos_message_queue_poller