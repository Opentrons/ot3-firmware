#pragma once

#include "can/core/messages.hpp"
#include "pipettes/core/tasks/eeprom_task.hpp"

namespace eeprom_message_handler {

template <eeprom_task::TaskClient EEPromTaskClient>
class EEPromHandler {
  public:
    using MessageType =
        std::variant<std::monostate, can_messages::WriteToEEPromRequest,
                     can_messages::ReadFromEEPromRequest>;

    explicit EEPromHandler(EEPromTaskClient &client) : client(client) {}
    EEPromHandler(const EEPromHandler &) = delete;
    EEPromHandler(const EEPromHandler &&) = delete;
    auto operator=(const EEPromHandler &) -> EEPromHandler & = delete;
    auto operator=(const EEPromHandler &&) -> EEPromHandler && = delete;
    ~EEPromHandler() = default;

    void handle(MessageType &m) { client.send_eeprom_queue(m); }

  private:
    EEPromTaskClient &client;
};

}  // namespace eeprom_message_handler
