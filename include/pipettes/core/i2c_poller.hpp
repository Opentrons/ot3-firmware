#pragma once

#include <cstdint>
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
     * @param number_reads: how many data points to take from the sensor
     * @param delay: the frequency to send the read/writes to the register
     * provided.
     * @param client_callback: the function that will write a CAN return message
     * to the client queue.
     * @param single_callback: the function that will handle any data
     * manipulation required for the specific task using the i2c bus.
     * @param reg: the register to read from, if relevant.
     *
     * A poll function automatically initiates a read/write to the specified
     * register on the i2c bus.
     */
    void single_register_poll(uint16_t device_address, uint16_t number_reads,
                              uint16_t delay,
                              SendToCanFunctionTypeDef client_callback,
                              SingleBufferTypeDef handle_callback,
                              uint8_t reg = 0x0) {
        pipette_messages::SingleRegisterPollReadFromI2C read_msg{
            .address = device_address,
            .polling = number_reads,
            .delay_ms = delay,
            .register_addr = reg,
            .client_callback = std::move(client_callback),
            .handle_buffer = std::move(handle_callback)};
        queue->try_write(read_msg);
    }

    /**
     * Multi Register Poll.
     *
     * This function will poll the results of two registers. Typically this is
     * relevant when data is stored in two separate registers.
     *
     * @param device_address: device address on the i2c bus
     * @param number_reads: how many data points to take from the sensor
     * @param delay: the frequency to send the read/writes to the register
     * provided.
     * @param client_callback: the function that will write a CAN return message
     * to the client queue.
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
    void multi_register_poll(
        uint16_t device_address, uint16_t number_reads, uint16_t delay,
        sensor_callbacks::SendToCanFunctionTypeDef client_callback,
        sensor_callbacks::MultiBufferTypeDef handle_callback,
        uint8_t register_1 = 0x0, uint8_t register_2 = 0x0) {
        pipette_messages::MultiRegisterPollReadFromI2C read_msg{
            .address = device_address,
            .polling = number_reads,
            .delay_ms = delay,
            .register_1_addr = register_1,
            .register_2_addr = register_2,
            .client_callback = std::move(client_callback),
            .handle_buffer = std::move(handle_callback)};
        queue->try_write(read_msg);
    }

    /**
     * Ongoing Multi Register Poll.
     *
     * This function will poll the results of two registers until reconfigured
     * or stopped. Typically this is relevant when data is stored in two
     * separate registers.
     *
     * @param device_address: device address on the i2c bus
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
    void ongoing_multi_register_poll(
        uint16_t device_address, uint16_t delay, uint32_t id,
        sensor_callbacks::MultiBufferTypeDef handle_callback,
        uint8_t register_1 = 0x0, uint8_t register_2 = 0x0) {
        std::array<uint8_t, MAX_SIZE> buffer_1{register_1};
        std::array<uint8_t, MAX_SIZE> buffer_2{register_2};
        pipette_messages::ConfigureMultiRegisterContinuousPolling read_msg{
            .poll_id = id,
            .address = device_address,
            .delay_ms = delay,
            .register_buffer_1 = buffer_1,
            .register_buffer_2 = buffer_2,
            .handle_buffer = std::move(handle_callback)};
        queue->try_write(read_msg);
    }

    /**
     * Single Register Ongoing Poll.
     *
     * This function will poll the results of a single register until
     * reconfigured or stopped.
     *
     * @param device_address: device address on the i2c bus
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

    void ongoing_single_register_poll(
        uint16_t device_address, uint16_t delay, uint32_t id,
        sensor_callbacks::SingleBufferTypeDef handle_callback,
        uint8_t reg = 0x0) {
        std::array<uint8_t, MAX_SIZE> buffer{reg};
        pipette_messages::ConfigureSingleRegisterContinuousPolling read_msg{
            .poll_id = id,
            .address = device_address,
            .delay_ms = delay,
            .register_buffer_1 = buffer,
            .handle_buffer = std::move(handle_callback)};
        queue->try_write(read_msg);
    }

    void set_queue(QueueType* q) { queue = q; }

  private:
    QueueType* queue{nullptr};
    static constexpr auto MAX_SIZE = 5;
};

};  // namespace i2c_poller
