#pragma once

#include <functional>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/motor_driver.hpp"
#include "motor-control/core/motor_driver_config.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace spi_task {

using TaskMessage =

/**
 * The handler of motor driver messages
 */
template <message_writer_task::TaskClient CanClient>
class SpiMessageHandler {
  public:
    SpiMessageHandler(spi::SpiDeviceBase& driver)
        : driver{driver} {}
    SpiMessageHandler(const SpiMessageHandler& c) = delete;
    SpiMessageHandler(const SpiMessageHandler&& c) = delete;
    auto operator=(const SpiMessageHandler& c) = delete;
    auto operator=(const SpiMessageHandler&& c) = delete;
    ~SpiMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    void handle(std::monostate m) { static_cast<void>(m); }

    void handle(const spi_messages::TaskMessage& m) {
        LOG("Received SPI read request");
        auto success = driver.transmit_receive(m.txData, m.rxData);
        m.queue->try_write(SpiTransactResponse{
            m.rxData,
            success,
            m.require_response
        });
    }

    spi::SpiDeviceBase& driver;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl,
    message_writer_task::TaskClient CanClient>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class SPITask {
  public:
    using QueueType = QueueImpl<TaskMessage>;
    SPITask(QueueType& queue) : queue{queue} {}
    SPITask(const SPITask& c) = delete;
    SPITask(const SPITask&& c) = delete;
    auto operator=(const SPITask& c) = delete;
    auto operator=(const SPITask&& c) = delete;
    ~SPITask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(spi::SpiDeviceBase* driver) {
        auto handler = SpiMessageHandler{*driver};
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

}  // namespace motor_driver_task