#pragma once

#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "common/core/message_queue.hpp"
#include "presence_sensor/core/presence_sensor_messages.hpp"
#include "presence_sensor/core/tasks/presence_sensing_driver_task.hpp"

namespace presence_sensing_message_handler {

using namespace can_message_writer;
using namespace can_messages;

template <presence_sensing_driver_task::TaskClient Client>
template <class PresenceSensor>
class PresenceSensingHandler {
  public:
    PresenceSensingHandler(Clinet &task_client) : task_client{task_client} {}
    PresenceSensingHandler(const PresenceSensingHandler &) = delete;
    PresenceSensingHandler(const PresenceSensingHandler &&) = delete;
    auto operator=(const PresenceSensingHandler &)
        -> PresenceSensorHandler & = delete;
    auto operator=(const PresenceSensingHandler &&)
        -> PresenceSensingHandler && = delete;
    ~PresenceSensingHandler() = default;

    using MessageType = std::variant < std::monostate, ;
    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    MessageWriter &message_writer;
    PresenceSensor &presence_sensor;
    Client &task_client;
};

}  // namespace presence_sensing_message_handler