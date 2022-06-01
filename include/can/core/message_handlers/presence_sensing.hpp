#pragma once

#include "can/core/messages.hpp"
#include "common/core/message_queue.hpp"
#include "head/core/tasks/presence_sensing_driver_task.hpp"

namespace can::message_handlers::presence_sensing {

using namespace can_messages;

template <
    presence_sensing_driver_task::TaskClient PresenceSensingDriverTaskClient>
class PresenceSensingHandler {
  public:
    PresenceSensingHandler(PresenceSensingDriverTaskClient &task_client)
        : presence_sensing_client{task_client} {}
    PresenceSensingHandler(const PresenceSensingHandler &) = delete;
    PresenceSensingHandler(const PresenceSensingHandler &&) = delete;
    auto operator=(const PresenceSensingHandler &)
        -> PresenceSensingHandler & = delete;
    auto operator=(const PresenceSensingHandler &&)
        -> PresenceSensingHandler && = delete;
    ~PresenceSensingHandler() = default;

    using MessageType =
        std::variant<std::monostate, ReadPresenceSensingVoltageRequest,
                     AttachedToolsRequest>;
    void handle(const MessageType &m) {
        std::visit([this](auto &message) { handle_message(message); }, m);
    }

  private:
    PresenceSensingDriverTaskClient &presence_sensing_client;
    void handle_message(std::monostate &m) { static_cast<void>(m); }
    void handle_message(const auto &m) {
        presence_sensing_client.send_presence_sensing_driver_queue(m);
    }
};

}  // namespace presence_sensing_message_handler