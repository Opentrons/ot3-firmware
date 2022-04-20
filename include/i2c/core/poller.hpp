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

namespace poller {
using namespace messages;

using TaskMessage =
    std::variant<std::monostate, SingleRegisterPollRead, MultiRegisterPollRead,
                 ConfigureSingleRegisterContinuousPolling,
                 ConfigureMultiRegisterContinuousPolling, TransactionResponse>;

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class Poller {
    using QueueType = QueueImpl<TaskMessage>;

  public:
    Poller() = default;
    ~Poller() = default;
    Poller(const Poller&) = delete;
    auto operator=(const Poller&) -> Poller& = delete;
    Poller(Poller&&) = delete;
    auto operator=(Poller&&) -> Poller& = delete;

    /*
     * Single Register Poll.
     *
     * This function will poll the results of a single register.
     *
     * @param device_address: device address on the i2c bus
     * @param data: The data to write to stimulate the read. Can be either
     * a buffer or a uint8_t.
     * @param read_bytes: Count of bytes to read in each poll
     * @param number_reads: how many data points to take from the sensor
     * @param delay: the frequency to send the read/writes to the register
     * provided.
     * @param response_queue: A queue capable of handling
     * i2c::messages::TransactionResponse, which will get response messages
     * from each transaction in the poll. The final transaction will have its
     * is_completed_transaction member set.
     * @param id: An id for the poll, which will be carried through in the
     * id of the responses.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus. After each write and read, the caller gets
     * an i2c::messages::TransactionResponse. After the final transaction,
     * that response's id field will have its is_completed_poll flag set to
     * true.
     */
    template <std::ranges::range DataBuf, I2CResponseQueue RQType>
    void single_register_poll(uint16_t device_address, const DataBuf& data,
                              std::size_t read_bytes, uint16_t number_reads,
                              uint16_t delay, RQType& response_queue,
                              uint32_t id = 0) {
        MaxMessageBuffer buffer{};
        auto write_size = std::min(data.size(), buffer.size());
        std::copy_n(data.cbegin(), write_size, buffer.begin());
        SingleRegisterPollRead read_msg{
            .polling = number_reads,
            .delay_ms = delay,
            .first = {.address = device_address,
                      .bytes_to_read = std::min(read_bytes, buffer.size()),
                      .bytes_to_write = write_size,
                      .write_buffer = buffer},
            .id = {.token = id, .is_completed_poll = false},
            .response_writer = ResponseWriter(response_queue)};
        queue->try_write(read_msg);
    }
    template <typename Data, I2CResponseQueue RQType>
    requires std::is_integral_v<Data>
    void single_register_poll(uint16_t device_address, Data data,
                              std::size_t read_bytes, uint16_t number_reads,
                              uint16_t delay, RQType& response_queue,
                              uint32_t id = 0) {
        std::array<uint8_t, sizeof(Data)> buf{};
        static_cast<void>(
            bit_utils::int_to_bytes(data, buf.begin(), buf.end()));

        single_register_poll(device_address, buf, read_bytes, number_reads,
                             delay, response_queue, id);
    }

    /**
     * Multi Register Poll.
     *
     * This function will poll the results of two registers. Typically this is
     * relevant when data is stored in two separate registers.
     *
     * @param device_address: device address on the i2c bus
     * @param register_1_data: Data buf to stimulate first register read.
     * @param register_1_read_bytes: The number of bytes to read from register 1
     * @param register_2_data: Data buf to stimulate second register read.
     * @param register_2_read_bytes: The number of bytes to read from register 2
     * The two register data args must either both be buffers or both be the
     * same integer type.
     * @param number_reads: how many data points to take from the sensor
     * @param delay: the frequency to send the read/writes to the register
     * provided.
     * @param response_queue: A response queue capable of handling
     * i2c::messages::TransactionResponse messages sent from the poller task
     * with the data that has been polled.
     * @param id: An arbitrary caller-set identifier that can associate the
     * response values with commanded polls.
     *
     * Registers will be written to/read from in the order they are listed. This
     * means that `register_1` will always be read from before `register_2`.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus.
     */
    template <std::ranges::range DataBuf, I2CResponseQueue RQType>
    void multi_register_poll(uint16_t device_address, DataBuf register_1_data,
                             std::size_t register_1_read_bytes,
                             DataBuf register_2_data,
                             std::size_t register_2_read_bytes,
                             uint16_t number_reads, uint16_t delay,
                             RQType& response_queue, uint32_t id = 0) {
        MaxMessageBuffer register_1_buffer{};
        auto reg_1_size =
            std::min(register_1_data.size(), register_1_buffer.size());
        std::copy_n(register_1_data.cbegin(), reg_1_size,
                    register_1_buffer.begin());
        MaxMessageBuffer register_2_buffer{};
        auto reg_2_size =
            std::min(register_2_data.size(), register_2_buffer.size());
        std::copy_n(register_2_data.cbegin(), reg_2_size,
                    register_2_buffer.begin());

        MultiRegisterPollRead read_msg{
            .polling = number_reads,
            .delay_ms = delay,
            .id = {.token = id, .is_completed_poll = false},
            .first = {.address = device_address,
                      .bytes_to_read = std::min(register_1_read_bytes,
                                                register_1_buffer.size()),
                      .bytes_to_write = reg_1_size,
                      .write_buffer = register_1_buffer},
            .second = {.address = device_address,
                       .bytes_to_read = std::min(register_2_read_bytes,
                                                 register_1_buffer.size()),
                       .bytes_to_write = reg_2_size,
                       .write_buffer = register_2_buffer

            },
            .response_writer = ResponseWriter(response_queue)};
        queue->try_write(read_msg);
    }
    template <typename Data, I2CResponseQueue RQType>
    requires std::is_integral_v<Data>
    void multi_register_poll(uint16_t device_address, Data register_1_data,
                             std::size_t register_1_read_bytes,
                             Data register_2_data,
                             std::size_t register_2_read_bytes,
                             uint16_t number_reads, uint16_t delay,
                             RQType& response_queue, uint32_t id = 0) {
        std::array<uint8_t, sizeof(Data)> buf_1{};
        std::array<uint8_t, sizeof(Data)> buf_2{};
        static_cast<void>(bit_utils::int_to_bytes(register_1_data,
                                                  buf_1.begin(), buf_1.end()));
        static_cast<void>(bit_utils::int_to_bytes(register_2_data,
                                                  buf_2.begin(), buf_2.end()));
        return multi_register_poll(device_address, buf_1, register_1_read_bytes,
                                   buf_2, register_2_read_bytes, number_reads,
                                   delay, response_queue, id);
    }

    /**
     * Ongoing Multi Register Poll.
     *
     * This function will poll the results of two registers until reconfigured
     * or stopped. Typically this is relevant when data is stored in two
     * separate registers.
     *
     * @param device_address: device address on the i2c bus
     * @param register_1_data: data buffer for the first register
     * @param register_1_read_bytes: Amount of data to read from first register
     * @param register_2_data: data buffer for the second register
     * @param register_2_read_bytes: Amount of data to read from second register
     * The two register vals may be either both be buffers or both be the same
     * integral type.
     * @param delay: the frequency to send the read/writes to the register
     * provided. A delay of 0 stops the poll.
     * @param response_queue: A queue that can handle
     * i2c::messages::TransactionResponse that will be sent after each
     * transactions.
     * @param id: A generic id for this poll; future calls with this ID will
     * reconfigure this poll, and the id will be passed i nresponse messages.
     *
     * Registers will be written to/read from in the order they are listed. This
     * means that `register_1` will always be read from before `register_2`.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus.
     */
    template <std::ranges::range DataBuf, I2CResponseQueue RQType>
    void continuous_multi_register_poll(uint16_t device_address,
                                        const DataBuf& register_1_data,
                                        std::size_t register_1_read_bytes,
                                        const DataBuf& register_2_data,
                                        std::size_t register_2_read_bytes,
                                        uint16_t delay, RQType& response_queue,
                                        uint32_t id) {
        MaxMessageBuffer buffer_1{};
        auto buffer_1_write = std::min(buffer_1.size(), register_1_data.size());
        std::copy_n(register_1_data.cbegin(), buffer_1_write, buffer_1.begin());

        MaxMessageBuffer buffer_2{};
        auto buffer_2_write = std::min(register_2_data.size(), buffer_2.size());
        std::copy_n(register_2_data.cbegin(), buffer_2_write, buffer_2.begin());

        ConfigureMultiRegisterContinuousPolling poll_msg{
            .delay_ms = delay,
            .first = {.address = device_address,
                      .bytes_to_read =
                          std::min(register_1_read_bytes, buffer_1.size()),
                      .bytes_to_write = buffer_1_write,
                      .write_buffer = buffer_1},
            .second = {.address = device_address,
                       .bytes_to_read =
                           std::min(register_2_read_bytes, buffer_2.size()),
                       .bytes_to_write = buffer_2_write,
                       .write_buffer = buffer_2},
            .id = {.token = id, .is_completed_poll = false},
            .response_writer = ResponseWriter(response_queue)};
        queue->try_write(poll_msg);
    }

    template <typename Data, I2CResponseQueue RQType>
    requires std::is_integral_v<Data>
    void continuous_multi_register_poll(uint16_t device_address,
                                        Data register_1_data,
                                        std::size_t register_1_read_bytes,
                                        Data register_2_data,
                                        std::size_t register_2_read_bytes,
                                        uint16_t delay, RQType& response_queue,
                                        uint32_t id) {
        std::array<uint8_t, sizeof(Data)> buf_1{};
        std::array<uint8_t, sizeof(Data)> buf_2{};
        static_cast<void>(bit_utils::int_to_bytes(register_1_data,
                                                  buf_1.begin(), buf_1.end()));
        static_cast<void>(bit_utils::int_to_bytes(register_2_data,
                                                  buf_2.begin(), buf_2.end()));
        continuous_multi_register_poll(
            device_address, buf_1, register_1_read_bytes, buf_2,
            register_2_read_bytes, delay, response_queue, id);
    }

    /**
     * Single Register Ongoing Poll.
     *
     * This function will poll the results of a single register until
     * reconfigured or stopped.
     *
     * @param device_address: device address on the i2c bus
     * @param data: data buffer to timulate register reading. May be
     * either a buffer or a uint8_t
     * @param read_bytes: Number of bytes to read from the device
     * @param delay: the frequency to send the read/writes to the register
     * provided. A delay of 0 stops the polling.
     * @param response_queue: A queue that can handle
     * i2c::messages::TransactionResponse and receive the outcome of the
     * polling transactions.
     * @param id: A generic id for this poll; future calls with this ID will
     * reconfigure this poll and the transaction response messages will
     * also contain it.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus.
     */
    template <std::ranges::range DataBuf, I2CResponseQueue RQType>
    void continuous_single_register_poll(uint16_t device_address,
                                         const DataBuf& data,
                                         std::size_t read_bytes, uint16_t delay,
                                         RQType& response_queue, uint32_t id) {
        MaxMessageBuffer buffer{};
        auto write_size = std::min(data.size(), buffer.size());
        std::copy_n(data.cbegin(), write_size, buffer.begin());
        ConfigureSingleRegisterContinuousPolling read_msg{
            .delay_ms = delay,
            .first = {.address = device_address,
                      .bytes_to_read = std::min(read_bytes, buffer.size()),
                      .bytes_to_write = write_size,
                      .write_buffer = buffer},
            .id = {.token = id, .is_completed_poll = false},
            .response_writer = ResponseWriter(response_queue)};
        queue->try_write(read_msg);
    }

    template <typename Data, I2CResponseQueue RQType>
    requires std::is_integral_v<Data>
    void continuous_single_register_poll(uint16_t device_address, Data data,
                                         std::size_t read_bytes, uint16_t delay,
                                         RQType& response_queue, uint32_t id) {
        std::array<uint8_t, sizeof(Data)> buf{};
        static_cast<void>(
            bit_utils::int_to_bytes(data, buf.begin(), buf.end()));
        continuous_single_register_poll(device_address, buf, read_bytes, delay,
                                        response_queue, id);
    }

    void set_queue(QueueType* q) { queue = q; }

  private:
    QueueType* queue{nullptr};
};
};  // namespace poller
};  // namespace i2c
