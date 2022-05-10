#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/messages.hpp"
#include "task.hpp"

namespace eeprom {
namespace message_handler {

using MessageType =
    std::variant<std::monostate, can_messages::WriteToEEPromRequest,
                 can_messages::ReadFromEEPromRequest>;

template <eeprom::task::TaskClient EEPromTaskClient,
          message_writer_task::TaskClient CanClient>
class EEPromHandler {
  public:
    explicit EEPromHandler(EEPromTaskClient &client, CanClient &can_client)
        : client(client), can_client(can_client) {}
    EEPromHandler(const EEPromHandler &) = delete;
    EEPromHandler(const EEPromHandler &&) = delete;
    auto operator=(const EEPromHandler &) -> EEPromHandler & = delete;
    auto operator=(const EEPromHandler &&) -> EEPromHandler && = delete;
    ~EEPromHandler() = default;

    void handle(MessageType &can_message) {
        std::visit([this](auto &m) -> void { this->visit(m); }, can_message);
    }

  private:
    void visit(std::monostate &) {}

    /**
     * Handle a request to write to eeprom
     * @param can_msg The CAN message
     */
    void visit(can_messages::WriteToEEPromRequest &can_msg) {
        auto msg = eeprom::message::WriteEepromMessage{
            .memory_address = can_msg.address,
            .length = can_msg.data_length,
            .data{can_msg.data}};
        client.send_eeprom_queue(msg);
    }

    /**
     * Handle a request to read from eeprom
     * @param can_msg The CAN message
     */
    void visit(can_messages::ReadFromEEPromRequest &can_msg) {
        auto msg = eeprom::message::ReadEepromMessage{
            .memory_address = can_msg.address,
            .length = can_msg.data_length,
            .callback = callback,
            .callback_param = this};
        client.send_eeprom_queue(msg);
    }

    /**
     * Handle a callback from eeprom task that a read is complete.
     * @param msg The read results
     * @param param Param (this pointer)
     */
    static void callback(const eeprom::message::EepromMessage &msg,
                         void *param) {
        auto *self =
            reinterpret_cast<EEPromHandler<EEPromTaskClient, CanClient> *>(
                param);
        auto message = can_messages::ReadFromEEPromResponse::create(
            msg.memory_address, msg.data.cbegin(), msg.data.end());
        self->can_client.send_can_message(can_ids::NodeId::host, message);
    }

    EEPromTaskClient &client;
    CanClient &can_client;
};

}  // namespace message_handler
}  // namespace eeprom
