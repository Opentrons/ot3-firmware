#pragma once

#include "can/core/can_bus.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "common/core/message_queue.hpp"
#include "pipettes/core/eeprom.hpp"

namespace eeprom_message_handler {

using namespace can_bus;
using namespace can_message_writer;
using namespace eeprom;
using namespace can_messages;

template <EEPromPolicy I2CComm>
class EEPromHandler {
  public:
    using MessageType = std::variant<std::monostate, WriteToEEPromRequest,
                                     ReadFromEEPromRequest>;

    explicit EEPromHandler(MessageWriter &message_writer, I2CComm &i2c)
        : message_writer(message_writer), i2c(i2c) {}
    EEPromHandler(const EEPromHandler &) = delete;
    EEPromHandler(const EEPromHandler &&) = delete;
    auto operator=(const EEPromHandler &) -> EEPromHandler & = delete;
    auto operator=(const EEPromHandler &&) -> EEPromHandler && = delete;
    ~EEPromHandler() = default;

    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(WriteToEEPromRequest &m) { eeprom::write(i2c, m.serial_number); }

    void visit(ReadFromEEPromRequest &m) {
        const uint8_t serial_number = eeprom::read(i2c);
        auto message = ReadFromEEPromResponse{{}, serial_number};
        message_writer.write(NodeId::host, message);
    }

    MessageWriter &message_writer;
    I2CComm &i2c;
};

}  // namespace eeprom_message_handler
