/*
 * Interface for the firmware-specifc parts of the host comms task
 */
#pragma once
#include <tuple>

#include "FreeRTOS.h"
#include "common/core/message_queue.hpp"
#include "common/core/version.h"
#include "rear-panel/core/binary_parse.hpp"
#include "rear-panel/core/double_buffer.hpp"
#include "rear-panel/core/messages.hpp"
#include "rear-panel/core/queues.hpp"
#include "rear-panel/firmware/system_hardware.h"

namespace host_comms_task {

class HostCommMessageHandler {
  public:
    explicit HostCommMessageHandler() {}
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
    auto handle_message(rearpanel::messages::HostCommTaskMessage &m,
                        InputIt tx_into, InputLimit tx_limit) -> InputIt {
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

    auto may_connect() -> bool { return may_connect_latch; }

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
    auto visit_message(rearpanel::messages::Echo &msg, InputIt tx_into,
                       InputLimit tx_limit) -> InputIt {
        return msg.serialize(tx_into, tx_limit);
    }

    // Create and transmit a device info response that includes the version
    // information
    template <typename InputIt, typename InputLimit>
    requires std::forward_iterator<InputIt> &&
        std::sized_sentinel_for<InputLimit, InputIt>
    auto visit_message(rearpanel::messages::DeviceInfoRequest &msg,
                       InputIt tx_into, InputLimit tx_limit) -> InputIt {
        std::ignore = msg;
        const auto *ver_info = version_get();
        const auto *rev_info = revision_get();
        auto response = rearpanel::messages::DeviceInfoResponse{
            .length = rearpanel::messages::DeviceInfoResponse::get_length(),
            .version = ver_info->version,
            .flags = ver_info->flags,
            .primary_revision = rev_info->primary,
            .secondary_revision = rev_info->secondary};
        std::copy_n(&ver_info->sha[0], VERSION_SHORTSHA_SIZE,
                    response.shortsha.begin());
        return response.serialize(tx_into, tx_limit);
    }

    // transmit the ack
    template <typename InputIt, typename InputLimit>
    requires std::forward_iterator<InputIt> &&
        std::sized_sentinel_for<InputLimit, InputIt>
    auto visit_message(rearpanel::messages::Ack &msg, InputIt tx_into,
                       InputLimit tx_limit) -> InputIt {
        return msg.serialize(tx_into, tx_limit);
    }

    // transmit the ack_failed
    template <typename InputIt, typename InputLimit>
    requires std::forward_iterator<InputIt> &&
        std::sized_sentinel_for<InputLimit, InputIt>
    auto visit_message(rearpanel::messages::AckFailed &msg, InputIt tx_into,
                       InputLimit tx_limit) -> InputIt {
        return msg.serialize(tx_into, tx_limit);
    }

    // Shutdown the hardware and enter the bootloader the ack_failed
    template <typename InputIt, typename InputLimit>
    requires std::forward_iterator<InputIt> &&
        std::sized_sentinel_for<InputLimit, InputIt>
    auto visit_message(rearpanel::messages::EnterBootlader &msg,
                       InputIt tx_into, InputLimit tx_limit) -> InputIt {
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_system_queue(msg);
        may_connect_latch = false;
        auto resp = rearpanel::messages::Ack{};
        return visit_message(resp, tx_into, tx_limit);
    }

    template <typename InputIt, typename InputLimit>
    requires std::forward_iterator<InputIt> &&
        std::sized_sentinel_for<InputLimit, InputIt>
    auto visit_message(rearpanel::messages::EnterBootloaderResponse &msg,
                       InputIt tx_into, InputLimit tx_limit) -> InputIt {
        return msg.serialize(tx_into, tx_limit);
    }

    bool may_connect_latch = true;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<rearpanel::messages::HostCommTaskMessage>,
                      rearpanel::messages::HostCommTaskMessage>
class HostCommTask {
  public:
    using Messages = rearpanel::messages::HostCommTaskMessage;
    using QueueType = QueueImpl<rearpanel::messages::HostCommTaskMessage>;
    HostCommTask(QueueType &queue) : queue{queue}, handler{} {}
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
        rearpanel::messages::HostCommTaskMessage message{};
        queue.try_read(&message);
        // We should now be guaranteed to have a message, and can visit it to do
        // our actual work.
        return handler.handle_message(message, tx_into, tx_limit);
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }
    [[nodiscard]] auto may_connect() -> bool { return handler.may_connect(); }

  private:
    QueueType &queue;
    HostCommMessageHandler handler;
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient =
    requires(Client client, const rearpanel::messages::HostCommTaskMessage &m) {
    {client.send_host_comms_queue(m)};
};

}  // namespace host_comms_task
