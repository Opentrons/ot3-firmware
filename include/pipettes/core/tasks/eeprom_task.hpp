#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.hpp"
#include "common/core/message_queue.hpp"

namespace eeprom_task {

using TaskMessage =
    std::variant<std::monostate, can_messages::WriteToEEPromRequest,
                 can_messages::ReadFromEEPromRequest>;

template <message_writer_task::TaskClient CanClient>
struct EEPromCallback {
    CanClient &can_client;

    void operator()(uint8_t *buff_pt, uint16_t size) {
        uint16_t data = 0x0;
        auto *iter = buff_pt;
        // I might be overthinking this, but I think this is the only way
        // we can get the entire array from the buffer pointer. Technically
        // this should be the underlying array from read buffer, but I'm
        // not confident in this.
        auto *end =
            iter +  // NOLINT (cppcoreguidelines-pro-bounds-pointer-arithmetic)
            size;   // NOLINT (cppcoreguidelines-pro-bounds-pointer-arithmetic)
        iter = bit_utils::bytes_to_int(iter, end, data);

        auto message = can_messages::ReadFromEEPromResponse{{}, data};
        can_client.send_can_message(can_ids::NodeId::host, message);
    }
};

template <class I2CQueueWriter, message_writer_task::TaskClient CanClient>
class EEPromMessageHandler {
  public:
    explicit EEPromMessageHandler(I2CQueueWriter &i2c_writer,
                                  CanClient &can_client)
        : writer{i2c_writer}, can_client{can_client}, callback{can_client} {}
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
        std::array<uint8_t, SERIAL_NUMBER_SIZE> write_buffer{};
        writer.write(write_buffer, m.serial_number, DEVICE_ADDRESS);
    }

    void visit(can_messages::ReadFromEEPromRequest &m) {
        LOG("Received request to read serial number");
        writer.read(read_buffer, DEVICE_ADDRESS, callback);
    }

    static constexpr uint16_t DEVICE_ADDRESS = 0x1;
    static constexpr int SERIAL_NUMBER_SIZE = 0x2;
    std::array<uint8_t, SERIAL_NUMBER_SIZE> read_buffer{};
    I2CQueueWriter &writer;
    CanClient &can_client;
    EEPromCallback<CanClient> callback;
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