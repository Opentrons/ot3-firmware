#pragma once

#include "can/core/can_writer_task.hpp"
#include "i2c/core/hardware_iface.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/writer.hpp"

namespace i2c {

namespace tasks {
using namespace messages;

class I2CMessageHandler {
  public:
    I2CMessageHandler(hardware::I2CDeviceBase &i2c_device)
        : i2c_device{i2c_device} {}
    I2CMessageHandler(const I2CMessageHandler &) = delete;
    I2CMessageHandler(const I2CMessageHandler &&) = delete;
    auto operator=(const I2CMessageHandler &) -> I2CMessageHandler & = delete;
    auto operator=(const I2CMessageHandler &&) -> I2CMessageHandler && = delete;
    ~I2CMessageHandler() = default;

    void handle_message(writer::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &) {}

    void visit(Transact &m) {
        messages::MaxMessageBuffer read_buf{};
        if (m.transaction.bytes_to_write != 0) {
            i2c_device.central_transmit(
                m.transaction.write_buffer.data(),
                std::min(m.transaction.bytes_to_write,
                         m.transaction.write_buffer.size()),
                m.transaction.address, TIMEOUT);
        }
        if (m.transaction.bytes_to_read != 0) {
            i2c_device.central_receive(
                read_buf.data(),
                std::min(m.transaction.bytes_to_read, read_buf.size()),
                m.transaction.address, TIMEOUT);
        }
        static_cast<void>(m.response_writer.write(
            TransactionResponse{.id = m.id,
                                .bytes_read = m.transaction.bytes_to_read,
                                .read_buffer = read_buf}));
    }

    i2c::hardware::I2CDeviceBase &i2c_device;

    // Default timeout should be 60 seconds
    // freertos expects this time to be in milliseconds
    static constexpr auto TIMEOUT = 60000;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<writer::TaskMessage>, writer::TaskMessage>
class I2CTask {
  public:
    using Messages = writer::TaskMessage;
    using QueueType = QueueImpl<writer::TaskMessage>;
    I2CTask(QueueType &queue) : queue{queue} {}
    I2CTask(const I2CTask &c) = delete;
    I2CTask(const I2CTask &&c) = delete;
    auto operator=(const I2CTask &c) = delete;
    auto operator=(const I2CTask &&c) = delete;
    ~I2CTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(i2c::hardware::I2CDeviceBase *driver) {
        auto handler = I2CMessageHandler{*driver};
        // Figure out task messages for I2C queue
        writer::TaskMessage message{};
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
};  // namespace tasks

}  // namespace i2c
