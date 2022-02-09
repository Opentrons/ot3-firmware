#pragma once

#include "can/core/can_writer_task.hpp"
#include "common/core/i2c.hpp"
#include "pipettes/core/i2c_writer.hpp"

namespace i2c_task {

using namespace i2c_writer;

class I2CMessageHandler {
  public:
    I2CMessageHandler(i2c::I2CDeviceBase &i2c_device)
        : i2c_device{i2c_device} {}
    I2CMessageHandler(const I2CMessageHandler &) = delete;
    I2CMessageHandler(const I2CMessageHandler &&) = delete;
    auto operator=(const I2CMessageHandler &) -> I2CMessageHandler & = delete;
    auto operator=(const I2CMessageHandler &&) -> I2CMessageHandler && = delete;
    ~I2CMessageHandler() = default;

    void handle_message(TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(WriteToI2C &m) {
        i2c_device.central_transmit(m.buffer.data(), m.buffer.size(), m.address, TIMEOUT);
    }

    void visit(ReadFromI2C &m) {
        i2c_device.central_receive(m.buffer.data(), m.buffer.size(), m.address, TIMEOUT);
        m.client_callback(m.buffer);
    }
    i2c::I2CDeviceBase &i2c_device;

    // Default timeout should be 60 seconds
    // freertos expects this time to be in milliseconds
    static constexpr auto TIMEOUT = 60000;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class I2CTask {
  public:
    using QueueType = QueueImpl<TaskMessage>;
    I2CTask(QueueType &queue) : queue{queue} {}
    I2CTask(const I2CTask &c) = delete;
    I2CTask(const I2CTask &&c) = delete;
    auto operator=(const I2CTask &c) = delete;
    auto operator=(const I2CTask &&c) = delete;
    ~I2CTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(i2c::I2CDeviceBase *driver) {
        auto handler = I2CMessageHandler{*driver};
        // Figure out task messages for I2C queue
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

}  // namespace i2c_task