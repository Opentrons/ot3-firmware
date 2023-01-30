/*
 * Interface for the firmware-specifc parts of the host comms task
 */
#pragma once

#include "FreeRTOS.h"
#include "common/core/message_queue.hpp"
#include "rear-panel/core/double_buffer.hpp"
#include "rear-panel/core/messages.hpp"

namespace host_comms_task {

using TaskMessage =
    std::variant<std::monostate, messages::IncomingMessageFromHost>;

template <class ResposneQueue>
class HostCommMessageHandler {
  public:
    explicit HostCommMessageHandler(ResposneQueue &resp_queue)
        : resp_queue{resp_queue} {}
    HostCommMessageHandler(const HostCommMessageHandler &) = delete;
    HostCommMessageHandler(const HostCommMessageHandler &&) = delete;
    auto operator=(const HostCommMessageHandler &)
        -> HostCommMessageHandler & = delete;
    auto operator=(const HostCommMessageHandler &&)
        -> HostCommMessageHandler && = delete;
    ~HostCommMessageHandler() = default;

    template <typename InputIt, typename InputLimit>
    requires std::forward_iterator<InputIt> &&
        std::sized_sentinel_for<InputLimit, InputIt>
    auto handle_message(messages::HostCommTaskMessage &m, InputIt tx_into,
                        InputLimit tx_limit) -> InputIt {
        // we need a this-capturing lambda to pass on the call to our set of
        // member function overloads because otherwise we would need a pointer
        // to member function, and you can't really do that with variant visit.
        // we also need to current the transmit buffer details in.
        auto visit_helper = [this, tx_into, tx_limit](auto &m) -> InputIt {
            return this->visit_message(m, tx_into, tx_limit);
        };

        // now, calling visit on the visit helper will pass through the calls to
        // our message handlers, and will pass through whatever the messages
        // return (aka how much data they wrote, if any) to the caller.
        return std::visit(visit_helper, m);
    }

  private:
    template <typename InputIt, typename InputLimit>
    requires std::forward_iterator<InputIt> &&
        std::sized_sentinel_for<InputLimit, InputIt>
    auto visit_message(std::monostate &m, InputIt tx_into, InputLimit tx_limit)
        -> InputIt {
        static_cast<void>(m);
        static_cast<void>(tx_into);
        static_cast<void>(tx_limit);
        return tx_into;
    }

    // for this very basic first pass we just want to echo back any messages we
    // receive from the host
    template <typename InputIt, typename InputLimit>
    requires std::forward_iterator<InputIt> &&
        std::sized_sentinel_for<InputLimit, InputIt>
    auto visit_message(messages::IncomingMessageFromHost &msg, InputIt tx_into,
                       InputLimit tx_limit) -> InputIt {
						   
        // TODO just doing this to echo when we build out the binary protocol in
        // RET-1304 we can do the parsing and handling with variants and that
        auto resp = messages::Echo{.length = uint16_t(tx_limit - tx_into),
                                   .data = msg.buffer};

        return visit_message(resp, tx_into, tx_limit);
    }

    // for this very basic first pass we just want to echo back any messages we
    // receive from the host
    template <typename InputIt, typename InputLimit>
    requires std::forward_iterator<InputIt> &&
        std::sized_sentinel_for<InputLimit, InputIt>
    auto visit_message(messages::Echo &msg, InputIt tx_into,
                       InputLimit tx_limit) -> InputIt {
        return std::copy(
            msg.data,
            std::min(msg.data + msg.length, msg.data + (tx_limit - tx_into)),
            tx_into);
    }

    ResposneQueue &resp_queue;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<messages::HostCommTaskMessage>,
                      messages::HostCommTaskMessage>
class HostCommTask {
  public:
    using Messages = messages::HostCommTaskMessage;
    using QueueType = QueueImpl<messages::HostCommTaskMessage>;
    HostCommTask(QueueType &queue) : queue{queue} {}
    HostCommTask(const HostCommTask &c) = delete;
    HostCommTask(const HostCommTask &&c) = delete;
    auto operator=(const HostCommTask &c) = delete;
    auto operator=(const HostCommTask &&c) = delete;
    ~HostCommTask() = default;

    /**
     * run_once() runs one spin of the task. This means it
     * - waits for a message to come in on its queue (either from another task,
     *or from the USB input handling machinery)
     * - handles the message
     *   - which may include sending other messages
     *   - which may include writing back a response string
     *
     * A buffer for the response string is provided by the caller, and it's
     *sadly provided as a c-style pointer+length pair because that's
     *fundamentally what it is. We could wrap it as an iterator pair, but it's
     *nice to be honest.
     *
     * This function returns the amount of data it actually wrote into tx_into.
     **/

    template <typename InputIt, typename InputLimit>
    requires std::forward_iterator<InputIt> &&
        std::sized_sentinel_for<InputLimit, InputIt>
    auto run_once(InputIt tx_into, InputLimit tx_limit) -> InputLimit {
        auto handler = HostCommMessageHandler{get_queue()};
        messages::HostCommTaskMessage message{};
        queue.try_read(&message);
        // We should now be guaranteed to have a message, and can visit it to do
        // our actual work.
        return handler.handle_message(message, tx_into, tx_limit);
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client,
                              const messages::HostCommTaskMessage &m) {
    {client.send_host_comms_queue(m)};
};

}  // namespace host_comms_task
