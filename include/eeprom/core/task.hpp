#pragma once

#include "common/core/bit_utils.hpp"
#include "common/core/buffer_type.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "common/core/message_utils.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/writer.hpp"
#include "messages.hpp"
#include "types.hpp"

namespace eeprom {
namespace task {

using TaskMessage =
    std::variant<eeprom::message::WriteEepromMessage,
                 eeprom::message::ReadEepromMessage,
                 i2c::messages::TransactionResponse, std::monostate>;

template <class I2CQueueWriter, class OwnQueue>
class EEPromMessageHandler {
  public:
    explicit EEPromMessageHandler(I2CQueueWriter &i2c_writer,
                                  OwnQueue &own_queue)
        : writer{i2c_writer}, own_queue{own_queue} {}
    EEPromMessageHandler(const EEPromMessageHandler &) = delete;
    EEPromMessageHandler(const EEPromMessageHandler &&) = delete;
    auto operator=(const EEPromMessageHandler &)
        -> EEPromMessageHandler & = delete;
    auto operator=(const EEPromMessageHandler &&)
        -> EEPromMessageHandler && = delete;
    ~EEPromMessageHandler() = default;

    void handle_message(TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &) {}

    void visit(i2c::messages::TransactionResponse &m) {
        auto data = eeprom::types::EepromData{};
        std::copy_n(m.read_buffer.cbegin(), std::min(m.bytes_read, data.size()),
                    data.begin());
        auto v = eeprom::message::EepromMessage{
            .memory_address = last_.memory_address,
            .length = static_cast<eeprom::types::data_length>(m.bytes_read),
            .data = data};
        last_.callback(v, last_.callback_param);
    }

    void visit(eeprom::message::WriteEepromMessage &m) {
        LOG("Received request to write %d bytes to address %x", m.length,
            m.data);
        auto buffer = i2c::messages::MaxMessageBuffer{};
        auto iter = buffer.begin();
        // First byte is address
        *iter++ = m.memory_address;
        // Remainder is data
        iter = std::copy_n(
            m.data.cbegin(),
            std::min(buffer.size() - 1, static_cast<std::size_t>(m.length)),
            iter);

        auto transaction = i2c::messages::Transaction{
            .address = DEVICE_ADDRESS,
            .bytes_to_read = 0,
            .bytes_to_write = static_cast<std::size_t>(iter - buffer.begin()),
            .write_buffer = buffer};
        auto transaction_id = i2c::messages::TransactionIdentifier{.token = 0};

        if (writer.transact(transaction, transaction_id, own_queue)) {
        }
    }

    void visit(eeprom::message::ReadEepromMessage &m) {
        if (m.length <= 0) {
            return;
        }
        LOG("Received request to read %d bytes from address %d", m.length,
            m.memory_address);

        auto transaction =
            i2c::messages::Transaction{.address = DEVICE_ADDRESS,
                                       .bytes_to_read = m.length,
                                       .bytes_to_write = 1,
                                       .write_buffer{m.memory_address}};
        auto transaction_id = i2c::messages::TransactionIdentifier{.token = 0};

        if (writer.transact(transaction, transaction_id, own_queue)) {
            last_ = m;
        }
    }

    static constexpr uint16_t DEVICE_ADDRESS = 0xA0;
    I2CQueueWriter &writer;
    OwnQueue &own_queue;

    ///
    eeprom::message::ReadEepromMessage last_{};
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class EEPromTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    EEPromTask(QueueType &queue) : queue{queue} {}
    EEPromTask(const EEPromTask &c) = delete;
    EEPromTask(const EEPromTask &&c) = delete;
    auto operator=(const EEPromTask &c) = delete;
    auto operator=(const EEPromTask &&c) = delete;
    ~EEPromTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(i2c::writer::Writer<QueueImpl> *writer) {
        auto handler = EEPromMessageHandler{*writer, get_queue()};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage &m) {
    {client.send_eeprom_queue(m)};
};

}  // namespace task
}  // namespace eeprom
