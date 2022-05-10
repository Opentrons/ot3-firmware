#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "task.hpp"

namespace eeprom {
namespace message_handler {

template <eeprom::task::TaskClient EEPromTaskClient>
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

    void handle(MessageType &) {}

  private:
    EEPromTaskClient &client;
};

}  // namespace message_handler
}  // namespace eeprom
