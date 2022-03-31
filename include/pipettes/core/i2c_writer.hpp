#pragma once

#include <cstdint>
#include <variant>

#include "common/core/bit_utils.hpp"
#include "common/core/buffer_type.hpp"
#include "common/core/message_queue.hpp"
#include "pipettes/core/messages.hpp"
#include "sensors/core/callback_types.hpp"

namespace i2c_writer {

using namespace pipette_messages;
using namespace sensor_callbacks;

using TaskMessage =
    std::variant<std::monostate, WriteToI2C, ReadFromI2C, TransactWithI2C>;

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

    template <typename Data>
    requires std::is_integral_v<Data>
    void write(uint16_t device_address, uint8_t reg, Data data) {
        std::array<uint8_t, MAX_SIZE> max_buffer{};
        buffering(max_buffer, data, reg);
        pipette_messages::WriteToI2C write_msg{.address = device_address,
                                               .buffer = max_buffer};
        queue->try_write(write_msg);
    }

    /**
     * A single read to the i2c bus.
     *
     * @param device_address: device address on the i2c bus
     * @param client_callback: the function that will write a CAN return message
     * to the client queue.
     * @param handle_callback: the function that will handle any data
     * manipulation required for the specific task using the i2c bus. Takes in
     * two buffers.
     * @param reg: the register to read from, if relevant.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus.
     */
    void read(uint16_t device_address, SendToCanFunctionTypeDef client_callback,
              SingleBufferTypeDef handle_callback, uint8_t reg = 0x0) {
        std::array<uint8_t, MAX_SIZE> max_buffer{reg};
        pipette_messages::ReadFromI2C read_msg{
            .address = device_address,
            .buffer = max_buffer,
            .client_callback = client_callback,
            .handle_buffer = handle_callback};
        queue->try_write(read_msg);
    }

    void transact(uint16_t device_address,
                  SendToCanFunctionTypeDef client_callback,
                  SingleBufferTypeDef handle_callback, uint8_t reg = 0x0) {
        std::array<uint8_t, MAX_SIZE> max_buffer{reg};
        pipette_messages::TransactWithI2C read_msg{
            .address = device_address,
            .buffer = max_buffer,
            .client_callback = std::move(client_callback),
            .handle_buffer = std::move(handle_callback)};
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

    template <GenericByteBuffer Buffer, typename Data>
    requires std::is_integral_v<Data>
    void buffering(Buffer& buffer, Data& data, uint8_t reg) {
        auto* iter = buffer.begin();
        iter = bit_utils::int_to_bytes(reg, iter, buffer.end());
        iter = bit_utils::int_to_bytes(data, iter, buffer.end());
    }

  private:
    QueueType* queue{nullptr};
    static constexpr auto MAX_SIZE = 5;
};

}  // namespace i2c_writer
