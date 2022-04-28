#pragma once

#include "common/core/bit_utils.hpp"
#include "common/core/buffer_type.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "common/core/message_utils.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/writer.hpp"
#include "types.hpp"
#include "messages.hpp"

namespace eeprom {
namespace task {

using TaskMessage = std::variant<
    eeprom::message::WriteEepromMessage,
      eeprom::message::ReadEepromMessage,
      i2c::messages::TransactionResponse,
    std::monostate>;

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

    void handle_message(TaskMessage& m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &) {}

    void visit(i2c::messages::TransactionResponse& m) {
//        auto message = can_messages::ReadFromEEPromResponse::create(
//            0, m.read_buffer.cbegin(), m.read_buffer.cend());
    }

    void visit(eeprom::message::WriteEepromMessage& m) {
        LOG("Received request to write %d bytes to address %x", m.length,
            m.data);
        auto buffer = i2c::messages::MaxMessageBuffer{};
        auto iter = buffer.begin();
        // First byte is address
        *iter++ = m.memory_address;
        // Remainder is data
        iter = std::copy_n(m.data.cbegin(), std::min(buffer.size() - 1, static_cast<std::size_t>(m.length)), iter);

        writer.write(DEVICE_ADDRESS, std::span(buffer.begin(), iter));
    }

    void visit(eeprom::message::ReadEepromMessage& m) {
        LOG("Received request to read %d bytes from address %d", m.length,
            m.memory_address);
//        writer.transact(DEVICE_ADDRESS, 0, 2, own_queue);
    }

    static constexpr uint16_t DEVICE_ADDRESS = 0xA0;
    I2CQueueWriter &writer;
    OwnQueue &own_queue;
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
