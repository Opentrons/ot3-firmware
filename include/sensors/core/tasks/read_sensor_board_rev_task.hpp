#pragma once

#include "sensors/core/pie4ioe5.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace tasks {

using TaskMessage =
    std::variant<std::monostate, i2c::messages::TransactionResponse>;

template <class I2CQueueWriter, class OwnQueue>
class ReadSenorBoardHandler {
  public:
    explicit ReadSenorBoardHandler(I2CQueueWriter &i2c_writer, OwnQueue &own_queue)
        : i2c_writer{i2c_writer}, task_queue{own_queue} {
        // request gpio expander status register
        i2c_writer->read(pie4ioe4::ADDRESS, static_cast<uint8_t>(pie4ioe4::registers::input_status), 1,
                        task_queue);
    }
    ReadSenorBoardHandler(const ReadSenorBoardHandler &) = delete;
    ReadSenorBoardHandler(const ReadSenorBoardHandler &&) = delete;
    auto operator=(const ReadSenorBoardHandler &)
        -> ReadSenorBoardHandler & = delete;
    auto operator=(const ReadSenorBoardHandler &&)
        -> ReadSenorBoardHandler && = delete;
    ~ReadSenorBoardHandler() {}

    void handle_message(const TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    utils::SensorBoardRev get_rev() { return rev; }

  private:
    void visit(const std::monostate &) {}

    void visit(i2c::messages::TransactionResponse &m) {
        if (m.bytes_read == 1 &&
            m.read_buffer[0] == static_cast<uint8_t>(pie4ioe4::version_responses::VERSION_1)) {
            rev = utils::SensorBoardRev::VERSION_1;
        }
    }

    utils::SensorBoardRev rev = utils::SensorBoardRev::VERSION_0;
    I2CQueueWriter &i2c_writer;
    OwnQueue& task_queue;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class ReadSenorBoardTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    ReadSenorBoardTask(QueueType &queue)
        : queue{queue} {}
    ReadSenorBoardTask(const ReadSenorBoardTask &c) = delete;
    ReadSenorBoardTask(const ReadSenorBoardTask &&c) = delete;
    auto operator=(const ReadSenorBoardTask &c) = delete;
    auto operator=(const ReadSenorBoardTask &&c) = delete;
    ~ReadSenorBoardTask() = default;

    /**
     * Task entry point.
     */
    void operator()(
        i2c::writer::Writer<QueueImpl> *writer,
        sensors::hardware::SensorHardwareVersionSingleton* version_wrapper) {
        auto handler = ReadSenorBoardHandler(writer, get_queue());

        TaskMessage message{};
        if (queue.try_read(&message, 1000)) {
            handler.handle_message(message);
        }
        version_wrapper->set_board_rev(handler.get_rev());
        vTaskDelete(nullptr);
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
};

}  // namespace task
}  // namespace sensors
