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

    template <SpiResponseQueue RQType>
    auto read(uint8_t addr, uint32_t command_data, RQType& response_queue,
              uint8_t id = 0) -> bool {
        auto txBuffer =
            build_message(addr, spi::hardware::Mode::READ, command_data);
        TransactionIdentifier _transaction_id{
            .token = id,
            .command_type = static_cast<uint8_t>(spi::hardware::Mode::READ),
            .send_response = false};
        Transact message{.id = _transaction_id,
                         .transaction = {.txBuffer = txBuffer},
                         .response_writer = ResponseWriter(response_queue)};
        queue->try_write(message);

        message.id.send_response = true;
        queue->try_write(message);
        return true;
    }

    template <SpiResponseQueue RQType>
    auto write(uint8_t addr, uint32_t command_data, RQType& response_queue,
               uint8_t id = 0) -> bool {
        auto txBuffer =
            build_message(addr, spi::hardware::Mode::WRITE, command_data);
        TransactionIdentifier _transaction_id{
            .token = id,
            .command_type = static_cast<uint8_t>(spi::hardware::Mode::WRITE),
            .send_response = false};
        Transact message{.id = _transaction_id,
                         .transaction = {.txBuffer = txBuffer},
                         .response_writer = ResponseWriter(response_queue)};
        queue->try_write(message);
        return true;
    }

  private:
    utils::MaxMessageBuffer rxBuffer{0};
    QueueType* queue{nullptr};

    /**
     * @brief Build a message to send over SPI
     * @param[in] addr The address to write to
     * @param[in] mode The mode to use, either WRITE or READ
     * @param[in] val The contents to write to the address (0 if this is a read)
     * @return An array with the contents of the message, or nothing if
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