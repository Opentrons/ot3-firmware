#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "task.hpp"

namespace eeprom {
namespace message_handler {

template <eeprom::task::TaskClient EEPromTaskClient>
class EEPromHandler {
  public:
    using MessageType = eeprom::task::CanMessage;

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

}  // namespace message_handler
}  // namespace eeprom
