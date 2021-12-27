#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/messages.hpp"
#include "common/core/message_queue.hpp"
#include "pipettes/core/eeprom.hpp"

namespace eeprom_task {

using TaskMessage =
    std::variant<std::monostate, can_messages::WriteToEEPromRequest,
                 can_messages::ReadFromEEPromRequest>;

template <message_writer_task::TaskClient CanClient>
class EEPromMessageHandler {
  public:
    explicit EEPromMessageHandler(i2c::I2CDeviceBase &i2c,
                                  CanClient &can_client)
        : eeprom_writer{i2c}, can_client{can_client} {}
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
        eeprom::write(eeprom_writer, m.serial_number);
    }

    void visit(can_messages::ReadFromEEPromRequest &m) {
        const uint8_t serial_number = eeprom::read(eeprom_writer);
        auto message = can_messages::ReadFromEEPromResponse{{}, serial_number};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }

    eeprom::EEPromWriter eeprom_writer;
    CanClient &can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl,
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
    [[noreturn]] void operator()(i2c::I2CDeviceBase *driver,
                                 CanClient *can_client) {
        auto handler = EEPromMessageHandler{*driver, *can_client};
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
