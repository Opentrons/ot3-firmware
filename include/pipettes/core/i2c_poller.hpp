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

namespace i2c_poller {

using namespace pipette_messages;
using namespace sensor_callbacks;

using TaskMessage = std::variant<std::monostate, SingleRegisterPollReadFromI2C,
                                 MultiRegisterPollReadFromI2C,
                                 ConfigureSingleRegisterContinuousPolling,
                                 ConfigureMultiRegisterContinuousPolling>;

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class I2CPoller {
    using QueueType = QueueImpl<TaskMessage>;

  public:
    I2CPoller() = default;
    ~I2CPoller() = default;
    I2CPoller(const I2CPoller&) = delete;
    auto operator=(const I2CPoller&) -> I2CPoller& = delete;
    I2CPoller(I2CPoller&&) = delete;
    auto operator=(I2CPoller&&) -> I2CPoller& = delete;

    /*
     * Single Register Poll.
     *
     * This function will poll the results of a single register.
     *
     * @param device_address: device address on the i2c bus
     * @param data: The data to write to stimulate the read. Can be either
     * a buffer or a uint8_t.
     * @param number_reads: how many data points to take from the sensor
     * @param delay: the frequency to send the read/writes to the register
     * provided.
     * @param client_callback: the function that will write a CAN return message
     * to the client queue.
     * @param single_callback: the function that will handle any data
     * manipulation required for the specific task using the i2c bus.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus.
     */
    template <std::ranges::range DataBuf>
    void single_register_poll(uint16_t device_address, const DataBuf& data,
                              uint16_t number_reads, uint16_t delay,
                              SendToCanFunctionTypeDef client_callback,
                              SingleBufferTypeDef handle_callback) {
        MaxMessageBuffer buffer{};
        std::copy_n(data.cbegin(), std::min(data.size(), buffer.size()),
                    buffer.begin());
        pipette_messages::SingleRegisterPollReadFromI2C read_msg{
            .address = device_address,
            .polling = number_reads,
            .delay_ms = delay,
            .buffer = buffer,
            .client_callback = std::move(client_callback),
            .handle_buffer = std::move(handle_callback)};
        queue->try_write(read_msg);
    }
    template <typename Data>
    requires std::is_integral_v<Data>
    void single_register_poll(uint16_t device_address, Data data,
                              uint16_t number_reads, uint16_t delay,
                              SendToCanFunctionTypeDef client_callback,
                              SingleBufferTypeDef handle_callback) {
        std::array<uint8_t, sizeof(Data)> buf{};
        static_cast<void>(
            bit_utils::int_to_bytes(data, buf.begin(), buf.end()));

        single_register_poll(device_address, buf, number_reads, delay,
                             client_callback, handle_callback);
    }

    /**
     * Multi Register Poll.
     *
     * This function will poll the results of two registers. Typically this is
     * relevant when data is stored in two separate registers.
     *
     * @param device_address: device address on the i2c bus
     * @param register_1_data: Data buf to stimulate first register read.
     * @param register_2_data: Data buf to stimulate second register read.
     * The two register args must either both be buffers or both be uint8_t.
     * @param number_reads: how many data points to take from the sensor
     * @param delay: the frequency to send the read/writes to the register
     * provided.
     * @param client_callback: the function that will write a CAN return message
     * to the client queue.
     * @param multi_callback: the function that will handle any data
     * manipulation required for the specific task using the i2c bus. Takes in
     * two buffers.
     *
     * Registers will be written to/read from in the order they are listed. This
     * means that `register_1` will always be read from before `register_2`.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus.
     */
    template <std::ranges::range DataBuf>
    void multi_register_poll(
        uint16_t device_address, DataBuf register_1_data,
        DataBuf register_2_data, uint16_t number_reads, uint16_t delay,
        sensor_callbacks::SendToCanFunctionTypeDef client_callback,
        sensor_callbacks::MultiBufferTypeDef handle_callback) {
        MaxMessageBuffer register_1_buffer{};
        std::copy_n(register_1_data.cbegin(),
                    std::min(register_1_data.size(), register_1_buffer.size()),
                    register_1_buffer.begin());
        MaxMessageBuffer register_2_buffer{};
        std::copy_n(register_2_data.cbegin(),
                    std::min(register_2_data.size(), register_2_buffer.size()),
                    register_2_buffer.begin());

        pipette_messages::MultiRegisterPollReadFromI2C read_msg{
            .address = device_address,
            .polling = number_reads,
            .delay_ms = delay,
            .register_1_buffer = register_1_buffer,
            .register_2_buffer = register_2_buffer,
            .client_callback = std::move(client_callback),
            .handle_buffer = std::move(handle_callback)};
        queue->try_write(read_msg);
    }
    template <typename Data>
    requires std::is_integral_v<Data>
    void multi_register_poll(
        uint16_t device_address, Data register_1_data, Data register_2_data,
        uint16_t number_reads, uint16_t delay,
        sensor_callbacks::SendToCanFunctionTypeDef client_callback,
        sensor_callbacks::MultiBufferTypeDef handle_callback) {
        std::array<uint8_t, sizeof(Data)> buf_1{}, buf_2{};
        static_cast<void>(bit_utils::int_to_bytes(register_1_data,
                                                  buf_1.begin(), buf_1.end()));
        static_cast<void>(bit_utils::int_to_bytes(register_2_data,
                                                  buf_2.begin(), buf_2.end()));
        return multi_register_poll(device_address, buf_1, buf_2, number_reads,
                                   delay, client_callback, handle_callback);
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
     * @param register_2_data: data buffer for the second register
     * The two register vals may be either both be buffers or both be uint8.
     * @param delay: the frequency to send the read/writes to the register
     * provided. A delay of 0 stops the poll.
     * @param id: A generic id for this poll; future calls with this ID will
     * reconfigure this poll.
     * @param multi_callback: the function that will handle any data
     * manipulation required for the specific task using the i2c bus. Takes in
     * two buffers.
     * @param register_1: the register to read from, if relevant.
     * @param register_2: the register to read from, if relevant.
     *
     * Registers will be written to/read from in the order they are listed. This
     * means that `register_1` will always be read from before `register_2`.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus.
     */
    template <std::ranges::range DataBuf>
    void continuous_multi_register_poll(
        uint16_t device_address, const DataBuf& register_1_data,
        const DataBuf& register_2_data, uint16_t delay, uint32_t id,
        sensor_callbacks::MultiBufferTypeDef handle_callback) {
        MaxMessageBuffer buffer_1{};
        std::copy_n(register_1_data.cbegin(),
                    std::min(buffer_1.size(), register_1_data.size()),
                    buffer_1.begin());

        MaxMessageBuffer buffer_2{};
        std::copy_n(register_2_data.cbegin(),
                    std::min(register_2_data.size(), buffer_2.size()),
                    buffer_2.begin());

        pipette_messages::ConfigureMultiRegisterContinuousPolling poll_msg{
            .poll_id = id,
            .address = device_address,
            .delay_ms = delay,
            .register_1_buffer = buffer_1,
            .register_2_buffer = buffer_2,
            .handle_buffer = std::move(handle_callback)};
        queue->try_write(poll_msg);
    }

    template <typename Data>
    requires std::is_integral_v<Data>
    void continuous_multi_register_poll(
        uint16_t device_address, Data register_1_data, Data register_2_data,
        uint16_t delay, uint32_t id,
        sensor_callbacks::MultiBufferTypeDef handle_callback) {
        std::array<uint8_t, sizeof(Data)> buf_1{}, buf_2{};
        static_cast<void>(bit_utils::int_to_bytes(register_1_data,
                                                  buf_1.begin(), buf_1.end()));
        static_cast<void>(bit_utils::int_to_bytes(register_2_data,
                                                  buf_2.begin(), buf_2.end()));
        continuous_multi_register_poll(device_address, buf_1, buf_2, delay, id,
                                       handle_callback);
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
     * @param delay: the frequency to send the read/writes to the register
     * provided. A delay of 0 stops the polling.
     * @param id: A generic id for this poll; future calls with this ID will
     * reconfigure this poll.
     * @param single_callback: the function that will handle any data
     * manipulation required for the specific task using the i2c bus.
     * @param reg: the register to read from, if relevant.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus.
     */
    template <std::ranges::range DataBuf>
    void continuous_single_register_poll(
        uint16_t device_address, const DataBuf& data, uint16_t delay,
        uint32_t id, sensor_callbacks::SingleBufferTypeDef handle_callback) {
        MaxMessageBuffer buffer{};
        std::copy_n(data.cbegin(), std::min(data.size(), buffer.size()),
                    buffer.begin());
        pipette_messages::ConfigureSingleRegisterContinuousPolling read_msg{
            .poll_id = id,
            .address = device_address,
            .delay_ms = delay,
            .buffer = buffer,
            .handle_buffer = std::move(handle_callback)};
        queue->try_write(read_msg);
    }

    template <typename Data>
    requires std::is_integral_v<Data>
    void continuous_single_register_poll(
        uint16_t device_address, Data data, uint16_t delay, uint32_t id,
        sensor_callbacks::SingleBufferTypeDef handle_callback) {
        std::array<uint8_t, sizeof(Data)> buf{};
        static_cast<void>(
            bit_utils::int_to_bytes(data, buf.begin(), buf.end()));
        continuous_single_register_poll(device_address, buf, delay, id,
                                        handle_callback);
    }

    void set_queue(QueueType* q) { queue = q; }

  private:
    QueueType* queue{nullptr};
};

};  // namespace i2c_poller
