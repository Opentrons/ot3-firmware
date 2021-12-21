#pragma once

#include "can/core/dispatch.hpp"
#include "can/core/messages.hpp"
#include "pipettes/core/tasks/eeprom.hpp"

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

/**
 * Type short cut for creating dispatch parse target for the handler.
 */
template <eeprom_task::TaskClient EEPromTaskClient>
using DispatchTarget =
    can_dispatch::DispatchParseTarget<EEPromHandler<EEPromTaskClient>,
                                      can_messages::WriteToEEPromRequest,
                                      can_messages::ReadFromEEPromRequest>;

}  // namespace eeprom_message_handler
