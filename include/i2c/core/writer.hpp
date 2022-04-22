#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>
#include <variant>

#include "common/core/bit_utils.hpp"
#include "common/core/buffer_type.hpp"
#include "common/core/message_queue.hpp"
#include "i2c/core/messages.hpp"

namespace i2c {
namespace writer {
using TaskMessage = std::variant<std::monostate, messages::Transact>;
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class Writer {
    using QueueType = QueueImpl<TaskMessage>;

  public:
    Writer() = default;
    ~Writer() = default;
    Writer(const Writer&) = delete;
    auto operator=(const Writer&) -> Writer& = delete;
    Writer(Writer&&) = delete;
    auto operator=(Writer&&) -> Writer& = delete;

    /**
     * A single write to the i2c bus.
     *
     * @param device_address: Device address on the i2c bus
     *
     * Write may be called with several different argument sets, including
     * @param buf a std::range representing a buffer of data to send
     * or
     * @param data any integral type representing pre-packed data to send
     * or
     * @param reg a sub-address in the target device's register model, which
     * will be sent as the first data byte
     * @param data any integral type of pre-packed data
     *
     * In any case, the size of the data to be written is computed from the
     * size of the input.
     *
     * Writes do not incur transaction responses.
     * */
    template <std::ranges::range DataBuffer>
    void write(uint16_t device_address, const DataBuffer& buf) {
        messages::MaxMessageBuffer max_buffer{};
        auto write_size = std::min(buf.size(), max_buffer.size());
        std::copy_n(buf.cbegin(), write_size, max_buffer.begin());
        do_write(device_address, write_size, max_buffer);
    }
    template <typename Data>
    requires std::is_integral_v<Data>
    void write(uint16_t device_address, Data data) {
        messages::MaxMessageBuffer max_buffer{};
        static_cast<void>(bit_utils::int_to_bytes(data, max_buffer.begin(),
                                                  max_buffer.end()));
        do_write(device_address, sizeof(data), max_buffer);
    }
    template <typename Data>
    requires std::is_integral_v<Data>
    void write(uint16_t device_address, uint8_t reg, Data data) {
        messages::MaxMessageBuffer max_buffer{};
        auto* iter = max_buffer.begin();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *iter++ = reg;
        static_cast<void>(
            bit_utils::int_to_bytes(data, iter, max_buffer.end()));
        do_write(device_address, sizeof(data) + 1, max_buffer);
    }

    /**
     * A single read to the i2c bus.
     *
     * @param device_address: device address on the i2c bus
     * @param read_bytes: Amount of data to read from the bus
     * @param rq: A response queue - a queue that can handle
     * i2c::messages::TransactionResponse messages through its try_write_static
     * method - that will receive a response message after the transaction
     * completes
     * @param id: An integer used to associate this call with its response
     * message.
     *
     * The read function enqueues a bus read without an associated write.
     *
     * The data read off the bus is packed into an
     * i2c::messages::TransactionResponse message and sent to the response queue
     * once the transaction completes.
     */
    template <messages::I2CResponseQueue RQType>
    void read(uint16_t device_address, std::size_t read_bytes,
              RQType& response_queue, uint32_t id = 0) {
        messages::Transact message{
            .transaction = {.address = device_address,
                            .bytes_to_read =
                                std::min(read_bytes, messages::MAX_BUFFER_SIZE),
                            .bytes_to_write = 0,
                            .write_buffer{}},
            .id = {.token = id, .is_completed_poll = false},
            .response_writer = messages::ResponseWriter(response_queue)};
        queue->try_write(message);
    }

    /*
     * A full read+write transaction on the i2c bus.
     *
     * There are multiple overloads of the method that allow different kinds of
     * call parameters. All of them share
     * @param device_address: device address on the i2c bus
     * The rest of the signatures can be
     * @param buf A std::range of data to send
     * @param read_count: A count of data to read from the buf
     * @param response_queue: A queue that can handle
     * i2c::messages::TransactionResponse
     * @param transaction_id: An int to associate this call with its response
     * or
     * @param data: Any integral type representing pre-packed data
     * @param read_count: As other overloads
     * @param response_queue: As other overloads
     * @param transaction_id: As other overloads
     * or
     * @param write_bytes: Amount of data to send
     * @param buf: A maximum-size prepacked buffer that can write a specific
     * amount of data
     * @param read_bytes: As other overloads
     * @param id: A pre-created full i2c::messages::TransactionIdentifier to
     * associate this call with its response
     * @param response_queue: As other overloads.
     * or
     * @param txn: A full premade i2c::messages::Transaction
     * @param id: A full premade i2c::messages::TransactionIdentifier
     * @param response_queue: As other overloads
     *
     * Note: the final overload mostly exists to handle requests from the i2c
     * poller, which is passing along Transactions pulled from previously
     * generated messages; it probably isn't what other callers should use.
     *
     * When the transaction is complete, the data read from the bus (if any)
     * will be sent in an i2c::messages::TransactionResponse to the
     * response_queue. If the read_bytes argument was 0, there will be no
     * response message; in that case, also consider using write().
     */
    template <std::ranges::range DataBuf,
              messages::I2CResponseQueue ResponseQueue>
    void transact(uint16_t device_address, DataBuf buf, std::size_t read_count,
                  ResponseQueue& response_queue, uint32_t transaction_id = 0) {
        messages::MaxMessageBuffer max_buffer{};
        auto write_size = std::min(buf.size(), max_buffer.size());
        std::copy_n(buf.cbegin(), write_size, max_buffer.begin());
        transact(device_address, write_size, max_buffer, read_count,
                 messages::TransactionIdentifier{.token = transaction_id,
                                                 .is_completed_poll = false},
                 response_queue);
    }

    template <typename Data, messages::I2CResponseQueue ResponseQueue>
    requires std::is_integral_v<Data>
    void transact(uint16_t device_address, Data data, std::size_t read_count,
                  ResponseQueue& response_queue, uint32_t transaction_id = 0) {
        messages::MaxMessageBuffer max_buffer{};
        static_cast<void>(bit_utils::int_to_bytes(data, max_buffer.begin(),
                                                  max_buffer.end()));
        transact(device_address, sizeof(data), max_buffer, read_count,
                 messages::TransactionIdentifier{.token = transaction_id,
                                                 .is_completed_poll = false},
                 response_queue);
    }

    template <typename ResponseQueue>
    void transact(uint16_t device_address, std::size_t write_bytes,
                  const messages::MaxMessageBuffer& buf, std::size_t read_bytes,
                  messages::TransactionIdentifier id,
                  ResponseQueue& response_queue) {
        transact(
            messages::Transaction{
                .address = device_address,
                .bytes_to_read =
                    std::min(read_bytes, messages::MAX_BUFFER_SIZE),
                .bytes_to_write = std::min(write_bytes, buf.size()),
                .write_buffer = buf},
            id, response_queue);
    }

    template <typename ResponseQueue>
    void transact(const messages::Transaction& txn,
                  const messages::TransactionIdentifier& id,
                  ResponseQueue& response_queue) {
        queue->try_write(messages::Transact{
            .transaction = txn,
            .id = id,
            .response_writer = messages::ResponseWriter(response_queue)});
    }

    void set_queue(QueueType* q) { queue = q; }

  private:
    void do_write(uint16_t address, std::size_t write_bytes,
                  const messages::MaxMessageBuffer& buf) {
        queue->try_write(messages::Transact{
            .transaction = {.address = address,
                            .bytes_to_read = 0,
                            .bytes_to_write = std::min(write_bytes, buf.size()),
                            .write_buffer = buf},
            .id = {.token = 0, .is_completed_poll = false},
            .response_writer = messages::ResponseWriter()});
    }
    QueueType* queue{nullptr};
};

}  // namespace writer

}  // namespace i2c
