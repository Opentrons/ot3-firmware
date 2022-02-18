#pragma once

#include <cstdint>
#include <variant>

#include "common/core/bit_utils.hpp"
#include "common/core/buffer_type.hpp"
#include "common/core/message_queue.hpp"
#include "pipettes/core/messages.hpp"

namespace i2c_writer {

using namespace pipette_messages;

using TaskMessage =
    std::variant<std::monostate, WriteToI2C, ReadFromI2C, PollReadFromI2C>;

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class I2CWriter {
    using QueueType = QueueImpl<TaskMessage>;

  public:
    I2CWriter() = default;
    ~I2CWriter() = default;
    I2CWriter(const I2CWriter&) = delete;
    auto operator=(const I2CWriter&) -> I2CWriter& = delete;
    I2CWriter(I2CWriter&&) = delete;
    auto operator=(I2CWriter&&) -> I2CWriter& = delete;

    template <typename Data>
    requires std::is_integral_v<Data>
    void write(Data data, uint16_t device_address) {
        std::array<uint8_t, MAX_SIZE> max_buffer{};
        buffering(max_buffer, data);
        pipette_messages::WriteToI2C write_msg{.address = device_address,
                                               .buffer = max_buffer};
        queue->try_write(write_msg);
    }

    void read(uint16_t device_address,
              const Callback
                  callback,  // NOLINT (performance-unnecessary-value-param)
              uint8_t reg = 0x0) {
        // We want to copy the callback every time as opposed to passing a
        // reference to it from the eeprom task.
        std::array<uint8_t, MAX_SIZE> max_buffer{reg};
        pipette_messages::ReadFromI2C read_msg{.address = device_address,
                                               .buffer = max_buffer,
                                               .client_callback = callback};
        queue->try_write(read_msg);
    }

    void poll_read(
        uint16_t device_address, uint8_t number_reads, uint16_t delay,
        const Callback
            callback,  // NOLINT (performance-unnecessary-value-param)
        uint8_t reg = 0x0) {
        // We want to copy the callback every time as opposed to passing a
        // reference to it from the eeprom task.
        std::array<uint8_t, MAX_SIZE> max_buffer{reg};
        pipette_messages::PollReadFromI2C read_msg{.address = device_address,
                                                   .polling = number_reads,
                                                   .buffer = max_buffer,
                                                   .client_callback = callback,
                                                   .delay_ms = delay};
        queue->try_write(read_msg);
    }

    void set_queue(QueueType* q) { queue = q; }

    template <GenericByteBuffer Buffer, typename Data>
    requires std::is_integral_v<Data>
    void debuffering(Buffer& buffer, Data& data) {
        auto* iter = buffer.begin();
        iter = bit_utils::bytes_to_int(iter, buffer.end(), data);
    }

    template <GenericByteBuffer Buffer, typename Data>
    requires std::is_integral_v<Data>
    void buffering(Buffer& buffer, Data& data) {
        auto* iter = buffer.begin();
        iter = bit_utils::int_to_bytes(data, iter, buffer.end());
    }

  private:
    QueueType* queue{nullptr};
    static constexpr auto MAX_SIZE = 5;
};

}  // namespace i2c_writer