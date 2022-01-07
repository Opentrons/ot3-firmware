#pragma once

#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "common/core/message_queue.hpp"
#include "presence-sensing/core/tasks/presence_sensing_driver_task.hpp"

namespace presence_sensing_message_handler {

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
        std::variant<std::monostate, ReadPresenceSensingVoltageRequest>;
    void handle(MessageType &m) {
        presence_sensing_client.send_presence_sensing_driver_queue(m);
    }

  private:
    PresenceSensingDriverTaskClient &presence_sensing_client;
};

}  // namespace presence_sensing_message_handler