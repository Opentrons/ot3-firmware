#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "task.hpp"

namespace eeprom {
namespace message_handler {

template <eeprom::task::TaskClient EEPromTaskClient, message_writer_task::TaskClient CanClient>
class EEPromHandler {
  public:
    using MessageType =  std::variant<std::monostate, can_messages::WriteToEEPromRequest, can_messages::ReadFromEEPromRequest>;

    explicit EEPromHandler(EEPromTaskClient &client, CanClient& can_client) : client(client), can_client(can_client) {}
    EEPromHandler(const EEPromHandler &) = delete;
    EEPromHandler(const EEPromHandler &&) = delete;
    auto operator=(const EEPromHandler &) -> EEPromHandler & = delete;
    auto operator=(const EEPromHandler &&) -> EEPromHandler && = delete;
    ~EEPromHandler() = default;

    void handle(MessageType &can_message) {
        std::visit(
            [this](auto& m) -> void { this->visit(m); },
            can_message);
    }

  private:
    void visit(std::monostate&) {}

    void visit(can_messages::WriteToEEPromRequest&) {}

    void visit(can_messages::ReadFromEEPromRequest&) {}

    EEPromTaskClient &client;
    CanClient& can_client;
};

}  // namespace message_handler
}  // namespace eeprom
