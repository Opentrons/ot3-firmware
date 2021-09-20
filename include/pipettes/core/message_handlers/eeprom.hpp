#pragma once

#include "can/core/can_bus.hpp"
#include "common/core/message_queue.hpp"
#include "can/core/message_writer.hpp"
#include "pipettes/core/eeprom.hpp"

using namespace can_bus;
using namespace can_message_writer;
using namespace eeprom;

namespace eeprom_message_handler {

template <CanBusWriter Writer, EEPromPolicy I2CComm>
class EEPromHandler {
  public:
    using MessageType = std::variant<std::monostate, WriteToEEPromRequest, ReadFromEEPromRequest>;

    explicit EEPromHandler(MessageWriter<Writer>& message_writer, I2CComm &i2c) : message_writer(message_writer), i2c(i2c) {}
    EEPromHandler(const EEPromHandler &) = delete;
    EEPromHandler(const EEPromHandler &&) = delete;
    EEPromHandler &operator=(const EEPromHandler &) = delete;
    EEPromHandler &&operator=(const EEPromHandler &&) = delete;

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

    MessageWriter<Writer> & message_writer;
    I2CComm &i2c;
};

}