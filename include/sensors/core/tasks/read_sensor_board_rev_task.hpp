#pragma once

#include "sensors/core/pie4ioe5.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

using TaskMessage =
    std::variant<std::monostate, i2c::messages::TransactionResponse>;

template <class I2CQueueWriter, class OwnQueue>
class ReadSensorBoardHandler {
  public:
    explicit ReadSensorBoardHandler(I2CQueueWriter &i2c_writer,
                                    OwnQueue &own_queue)
        : i2c_writer{i2c_writer}, task_queue{own_queue} {
        // request gpio expander status register
        i2c_writer->read(
            pie4ioe4::ADDRESS,
            static_cast<uint8_t>(pie4ioe4::registers::input_status), 1,
            task_queue);
    }
    ReadSensorBoardHandler(const ReadSensorBoardHandler &) = delete;
    ReadSensorBoardHandler(const ReadSensorBoardHandler &&) = delete;
    auto operator=(const ReadSensorBoardHandler &)
        -> ReadSensorBoardHandler & = delete;
    auto operator=(const ReadSensorBoardHandler &&)
        -> ReadSensorBoardHandler && = delete;
    ~ReadSensorBoardHandler() {}

    void handle_message(const TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    utils::SensorBoardRev get_rev() { return rev; }

  private:
    void visit(const std::monostate &) {}

    void visit(i2c::messages::TransactionResponse &m) {
        if (m.bytes_read == 1 &&
            m.read_buffer[0] ==
                static_cast<uint8_t>(pie4ioe4::version_responses::VERSION_1)) {
            rev = utils::SensorBoardRev::VERSION_1;
        }
    }

    utils::SensorBoardRev rev = utils::SensorBoardRev::VERSION_0;
    I2CQueueWriter &i2c_writer;
    OwnQueue &task_queue;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class ReadSensorBoardTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    ReadSensorBoardTask(QueueType &queue) : queue{queue} {}
    ReadSensorBoardTask(const ReadSensorBoardTask &c) = delete;
    ReadSensorBoardTask(const ReadSensorBoardTask &&c) = delete;
    auto operator=(const ReadSensorBoardTask &c) = delete;
    auto operator=(const ReadSensorBoardTask &&c) = delete;
    ~ReadSensorBoardTask() = default;

    /**
     * Task entry point.
     */
    void operator()(
        i2c::writer::Writer<QueueImpl> *writer,
        sensors::hardware::SensorHardwareVersionSingleton *version_wrapper) {
        auto handler = ReadSensorBoardHandler(writer, get_queue());

        TaskMessage message{};
        if (queue.try_read(&message, queue.max_delay)) {
            handler.handle_message(message);
        }
        version_wrapper->set_board_rev(handler.get_rev());
        vTaskDelete(nullptr);
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
};

}  // namespace tasks
}  // namespace sensors
