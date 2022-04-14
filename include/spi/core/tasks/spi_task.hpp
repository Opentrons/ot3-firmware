#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "spi/core/messages.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/utils.hpp"

namespace spi {

namespace tasks {

using TaskMessage = std::variant<std::monostate, spi::messages::Transact>;
/**
 * The handler of motor driver messages
 */
class MessageHandler {
  public:
    MessageHandler(spi::hardware::SpiDeviceBase& driver) : driver{driver} {}
    MessageHandler(const MessageHandler& c) = delete;
    MessageHandler(const MessageHandler&& c) = delete;
    auto operator=(const MessageHandler& c) = delete;
    auto operator=(const MessageHandler&& c) = delete;
    ~MessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->visit(m); }, message);
    }

  private:
    void visit(std::monostate m) {}

    void visit(spi::messages::Transact& m) {
        LOG("Received SPI read request");
        spi::utils::MaxMessageBuffer rxBuffer{};
        auto success =
            driver.transmit_receive(m.transaction.txBuffer, rxBuffer);
        m.response_writer.write(spi::messages::TransactResponse{
            .id = m.id, .rxBuffer = rxBuffer, .success = success});
    }

    spi::hardware::SpiDeviceBase& driver;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class Task {
  public:
    using QueueType = QueueImpl<TaskMessage>;
    Task(QueueType& queue) : queue{queue} {}
    Task(const Task& c) = delete;
    Task(const Task&& c) = delete;
    auto operator=(const Task& c) = delete;
    auto operator=(const Task&& c) = delete;
    ~Task() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(spi::hardware::SpiDeviceBase* driver) {
        auto handler = MessageHandler{*driver};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
};

}  // namespace tasks

}  // namespace spi