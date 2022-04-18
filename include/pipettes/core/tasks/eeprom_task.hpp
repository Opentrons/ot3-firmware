#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/buffer_type.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "common/core/message_utils.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/writer.hpp"

namespace eeprom_task {

using _CanMessageTuple = std::tuple<can_messages::WriteToEEPromRequest,
                                    can_messages::ReadFromEEPromRequest>;
using CanMessage = typename utils::TuplesToVariants<std::tuple<std::monostate>,
                                                    _CanMessageTuple>::type;

using TaskMessage = typename utils::VariantCat<
    CanMessage, std::variant<i2c::messages::TransactionResponse>>::type;

template <class I2CQueueWriter, message_writer_task::TaskClient CanClient,
          class OwnQueue>
class EEPromMessageHandler {
  public:
    explicit EEPromMessageHandler(I2CQueueWriter &i2c_writer,
                                  CanClient &can_client, OwnQueue &own_queue)
        : writer{i2c_writer}, can_client{can_client}, own_queue{own_queue} {}
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
    void visit(std::monostate &m) {}

    void visit(i2c::messages::TransactionResponse &m) {
        uint16_t data = 0;
        static_cast<void>(bit_utils::bytes_to_int(m.read_buffer.cbegin(),
                                                  m.read_buffer.cend(), data));
        auto message = can_messages::ReadFromEEPromResponse({}, data);
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    void visit(can_messages::WriteToEEPromRequest &m) {
        LOG("Received request to write serial number: %d", m.serial_number);
        std::array serial_buf{static_cast<uint8_t>(m.serial_number >> 8),
                              static_cast<uint8_t>(m.serial_number & 0xff)};
        writer.write(DEVICE_ADDRESS, serial_buf);
    }

    void visit(can_messages::ReadFromEEPromRequest &m) {
        LOG("Received request to read serial number");
        writer.transact(DEVICE_ADDRESS, 0, 2, own_queue);
    }

    static constexpr uint16_t DEVICE_ADDRESS = 0x1;
    static constexpr int SERIAL_NUMBER_SIZE = 0x2;
    I2CQueueWriter &writer;
    CanClient &can_client;
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
    template <message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(i2c::writer::Writer<QueueImpl> *writer,
                                 CanClient *can_client) {
        auto handler = EEPromMessageHandler{*writer, *can_client, get_queue()};
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

}  // namespace eeprom_task
