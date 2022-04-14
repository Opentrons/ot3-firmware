#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "pipettes/core/tasks/eeprom_task.hpp"

namespace eeprom_message_handler {

template <eeprom_task::TaskClient EEPromTaskClient>
class EEPromHandler {
  public:
    using MessageType = eeprom_task::CanMessage;

    explicit EEPromHandler(EEPromTaskClient &client) : client(client) {}
    EEPromHandler(const EEPromHandler &) = delete;
    EEPromHandler(const EEPromHandler &&) = delete;
    auto operator=(const EEPromHandler &) -> EEPromHandler & = delete;
    auto operator=(const EEPromHandler &&) -> EEPromHandler && = delete;
    ~EEPromHandler() = default;

    void handle(MessageType &can_message) {
        std::visit(
            [this](auto m) -> void { this->client.send_eeprom_queue(m); },
            can_message);
    }

  private:
    EEPromTaskClient &client;
};

}  // namespace eeprom_message_handler
