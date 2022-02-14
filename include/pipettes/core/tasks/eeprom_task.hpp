#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/buffer_type.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "pipettes/core/messages.hpp"
#include "sensors/core/callback_types.hpp"

namespace eeprom_task {

using TaskMessage =
    std::variant<std::monostate, can_messages::WriteToEEPromRequest,
                 can_messages::ReadFromEEPromRequest>;

template <message_writer_task::TaskClient CanClient>
struct EEPromCallback {
  public:
    EEPromCallback(CanClient &can_client) : can_client{can_client} {}

    void handle_data(const sensor_callbacks::MaxMessageBuffer &buffer) {
        const auto *iter = buffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, buffer.cend(), data);
    }

    void send_to_can() {
        auto message = can_messages::ReadFromEEPromResponse{{}, data};
        can_client.send_can_message(can_ids::NodeId::host, message);
        data = 0;
    }

  private:
    CanClient &can_client;
    uint16_t data = 0;
};

template <class I2CQueueWriter, message_writer_task::TaskClient CanClient>
class EEPromMessageHandler {
  public:
    explicit EEPromMessageHandler(I2CQueueWriter &i2c_writer,
                                  CanClient &can_client)
        : writer{i2c_writer}, can_client{can_client}, handler{can_client} {}
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

    void visit(can_messages::WriteToEEPromRequest &m) {
        LOG("Received request to write serial number: %d\n", m.serial_number);
        writer.write(m.serial_number, DEVICE_ADDRESS);
    }

    void visit(can_messages::ReadFromEEPromRequest &m) {
        LOG("Received request to read serial number\n");
        writer.read(
            DEVICE_ADDRESS, [this]() { handler.send_to_can(); },
            [this](auto message_a) { handler.handle_data(message_a); });
    }

    static constexpr uint16_t DEVICE_ADDRESS = 0x1;
    static constexpr int SERIAL_NUMBER_SIZE = 0x2;
    I2CQueueWriter &writer;
    CanClient &can_client;
    EEPromCallback<CanClient> handler;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl, class I2CQueueWriter,
          message_writer_task::TaskClient CanClient>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class EEPromTask {
  public:
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
    [[noreturn]] void operator()(I2CQueueWriter *writer,
                                 CanClient *can_client) {
        auto handler = EEPromMessageHandler{*writer, *can_client};
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