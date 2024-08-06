#pragma once

#include <variant>

#include "common/core/message_queue.hpp"
#include "spi/core/messages.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/utils.hpp"

namespace spi {

namespace writer {

using namespace spi::messages;

using TaskMessage = std::variant<std::monostate, Transact>;

/**
 * Class that handles writing commands to a SPI task queue
 *
 * @tparam QueueImpl - the SPI task queue to write to
 */
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

    void set_queue(QueueType* q) { queue = q; }

    /**
     * @brief Command to add two transact message(s) to the SPI Task Queue
     *
     * @tparam[RQType] The originating response queue
     * @param[register_addr] The address to read from
     * @param[command_data] The contents to send to the address (generally 0)
     * @param[response_queue] The queue with which the SPI task will write a
     * response
     * @return A success boolean
     */
    template <OriginatingResponseQueue RQType>
    auto read(uint32_t token, uint32_t command_data, RQType& response_queue,
              utils::ChipSelectInterface cs_intf, uint32_t message_index)
        -> bool {
        auto txBuffer = build_message(utils::reg_from_token<uint8_t>(token),
                                      spi::hardware::Mode::READ, command_data);
        TransactionIdentifier _transaction_id{
            .token = token,
            .command_type = static_cast<uint8_t>(spi::hardware::Mode::READ),
            .requires_response = false,
            .message_index = message_index};
        Transact message{
            .id = _transaction_id,
            .transaction = {.txBuffer = txBuffer, .cs_interface = cs_intf},
            .response_writer = ResponseWriter(response_queue)};
        queue->try_write(message);

        message.id.requires_response = true;
        return queue->try_write(message);
    }

    /**
     * @brief Command to add a transact message to the SPI Task Queue
     *
     * @tparam[RQType] The originating response queue
     * @param[register_addr] The address to write to
     * @param[command_data] The contents to write to the address
     * @param[response_queue] The queue with which the SPI task will write a
     * response
     * @return A success boolean
     */
    template <OriginatingResponseQueue RQType>
    auto write(uint8_t register_addr, uint32_t command_data,
               RQType& response_queue, utils::ChipSelectInterface cs_intf,
               uint8_t timeout_ms = 1) -> bool {
        auto txBuffer = build_message(register_addr, spi::hardware::Mode::WRITE,
                                      command_data);
        TransactionIdentifier _transaction_id{
            .token = utils::build_token(register_addr),
            .command_type = static_cast<uint8_t>(spi::hardware::Mode::WRITE),
            .requires_response = true};
        Transact message{
            .id = _transaction_id,
            .transaction = {.txBuffer = txBuffer, .cs_interface = cs_intf},
            .response_writer = ResponseWriter(response_queue)};
        return queue->try_write(message, timeout_ms);
    }

  private:
    utils::MaxMessageBuffer rxBuffer{0};
    QueueType* queue{nullptr};

    /**
     * @brief Build a message to send over SPI
     *
     * @param[in] addr The address to write to
     * @param[in] mode The mode to use, either WRITE or READ
     * @param[in] val The contents to write to the address (0 if this is a read)
     * @return An array with the contents of the message, or an empty buffer if
     * there was an error
     */
    static auto build_message(uint8_t addr, spi::hardware::Mode mode,
                              uint32_t val) -> utils::MaxMessageBuffer {
        utils::MaxMessageBuffer buffer = {0};
        auto* iter = buffer.begin();
        auto addr_byte = addr;
        addr_byte |= static_cast<uint8_t>(mode);
        iter = bit_utils::int_to_bytes(addr_byte, iter, buffer.end());
        iter = bit_utils::int_to_bytes(val, iter, buffer.end());
        if (iter != buffer.end()) {
            return utils::MaxMessageBuffer();
        }
        return buffer;
    }
};

}  // namespace writer

}  // namespace spi
