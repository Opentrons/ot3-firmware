#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>
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

    /**
     * A single write to the i2c bus.
     *
     * @param device_address: Device address on the i2c bus
     * @param buf: prepared buffer for writing or int to serialize
     * */
    template <std::ranges::range DataBuffer>
    void write(uint16_t device_address, const DataBuffer& buf) {
        MaxMessageBuffer max_buffer{};
        std::copy_n(buf.cbegin(), std::min(buf.size(), max_buffer.size()),
                    max_buffer.begin());
        do_write(device_address, max_buffer);
    }

    template <typename Data>
    requires std::is_integral_v<Data>
    void write(uint16_t device_address, Data data) {
        MaxMessageBuffer max_buffer{};
        static_cast<void>(bit_utils::int_to_bytes(data, max_buffer.begin(),
                                                  max_buffer.end()));
        do_write(device_address, max_buffer);
    }

    template <typename Data>
    requires std::is_integral_v<Data>
    void write(uint16_t device_address, uint8_t reg, Data data) {
        MaxMessageBuffer max_buffer{};
        auto iter = max_buffer.begin();
        *iter++ = reg;
        static_cast<void>(
            bit_utils::int_to_bytes(data, iter, max_buffer.end()));
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
     * a buffer containing the data.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus.
     */
    void read(uint16_t device_address, SendToCanFunctionTypeDef client_callback,
              SingleBufferTypeDef handle_callback) {
        MaxMessageBuffer max_buffer{};
        pipette_messages::ReadFromI2C read_msg{
            .address = device_address,
            .buffer = max_buffer,
            .client_callback = client_callback,
            .handle_buffer = handle_callback};
        queue->try_write(read_msg);
    }

    /*
     * A transaction on the i2c bus with writing and reading.
     *
     * @param device_address: device address on the i2c bus
     * @param data: Data to write to the bus before reading. Can be a full
     * buffer or a uint8_t.
     * @param client_callback: the function that will write a CAN return message
     * to the client queue.
     * @param handle_callback: the funciton that will handle any data
     * manipulation required for the specific task using the i2c bus. Takes in
     * a buffer.
     * @param reg: The register to read from.
     */
    template <std::ranges::range DataBuf>
    void transact(uint16_t device_address, DataBuf buf,
                  const SendToCanFunctionTypeDef& client_callback,
                  const SingleBufferTypeDef& handle_callback) {
        MaxMessageBuffer max_buffer{};
        std::copy_n(buf.cbegin(), std::min(buf.size(), max_buffer.size()),
                    max_buffer.begin());
        do_transact(device_address, max_buffer, client_callback,
                    handle_callback);
    }

    template <typename Data>
    requires std::is_integral_v<Data>
    void transact(uint16_t device_address, Data data,
                  const SendToCanFunctionTypeDef& client_callback,
                  const SingleBufferTypeDef& handle_callback) {
        MaxMessageBuffer max_buffer{};
        static_cast<void>(bit_utils::int_to_bytes(data, max_buffer.begin(),
                                                  max_buffer.end()));
        do_transact(device_address, max_buffer, client_callback,
                    handle_callback);
    }

    void set_queue(QueueType* q) { queue = q; }

  private:
    void do_transact(uint16_t address, const MaxMessageBuffer& buf,
                     const SendToCanFunctionTypeDef& client_callback,
                     const SingleBufferTypeDef& handle_callback) {
        pipette_messages::TransactWithI2C read_msg{
            .address = address,
            .buffer = buf,
            .client_callback = client_callback,
            .handle_buffer = handle_callback};
        queue->try_write(read_msg);
    }
    void do_write(uint16_t address, const MaxMessageBuffer& buf) {
        pipette_messages::WriteToI2C write_msg{.address = address,
                                               .buffer = buf};
        queue->try_write(write_msg);
    }
    QueueType* queue{nullptr};
};

}  // namespace i2c_writer
