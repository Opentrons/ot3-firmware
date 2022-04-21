#pragma once

#include "common/core/message_queue.hpp"
#include "spi/core/utils.hpp"

namespace spi {

namespace messages {

/**
 *
 * @brief Transaction identifier struct
 *
 * @param[token] id of the message (generally the register reading/writing to)
 * @param[command_type] whether it's a read or write command
 * @param[requires_response] whether the message requires a response
 */
struct TransactionIdentifier {
    uint32_t token;
    uint8_t command_type;
    bool requires_response;

    auto operator==(const TransactionIdentifier& other) const -> bool = default;
};

/**
 * @brief Transaction, the contents of a transaction request
 *
 * @param[txBuffer] The buffer to be transmitted
 */
struct Transaction {
    spi::utils::MaxMessageBuffer txBuffer;

    auto operator==(const Transaction&) const -> bool = default;
};

/**
 *
 * @brief TransactResponse, the message sent back from the SPI task
 *
 * @param[id] Originating message ID
 * @param[rxBuffer] The response from transact
 * @param[success] A boolean recording whether the SPI transact was successful
 */
struct TransactResponse {
    auto operator==(const TransactResponse&) const -> bool = default;
    TransactionIdentifier id;
    spi::utils::MaxMessageBuffer rxBuffer;
    bool success;
};

/**
 * A concept that can be used to identify a queue capable of receiving a
 * Transaction Response.
 */
template <typename MessageQueue>
concept OriginatingResponseQueue =
    RespondableMessageQueue<MessageQueue, TransactResponse>;

/**
 * Holds a special tiny little closure for writing response values
 * that can be passed something with static lifetime - a normal
 * function pointer. We need this because it will be memcpy'd, and
 * so we can't use an actual safe closure like a std::function because
 * it will get destroyed.
 */
struct ResponseWriter {
    template <OriginatingResponseQueue OriginatingQueue>
    explicit ResponseWriter(OriginatingQueue& rq)
        : queue_ref(static_cast<void*>(&rq)),
          writer(&OriginatingQueue::try_write_static) {}
    ResponseWriter(const ResponseWriter& other) = default;
    auto operator=(const ResponseWriter& other) -> ResponseWriter& = default;
    ResponseWriter(ResponseWriter&& other) = default;
    auto operator=(ResponseWriter&& other) -> ResponseWriter& = default;
    ResponseWriter() = default;
    ~ResponseWriter() = default;
    void* queue_ref{nullptr};
    bool (*writer)(void*, const TransactResponse&){nullptr};

    // NOLINTNEXTLINE(modernize-use-nodiscard)
    auto write(const TransactResponse& response) const -> bool {
        if (bool(writer) && bool(queue_ref)) {
            return writer(queue_ref, response);
        }
        return false;
    }
};

/**
 *
 * @brief Transact, the full message sent to the SPI task queue
 *
 * @param[id] Originating message ID
 * @param[transaction] The response from transact
 * @param[response_writer] A closure containing the originating task queue
 */
struct Transact {
    TransactionIdentifier id;
    Transaction transaction;
    ResponseWriter response_writer;
};

}  // namespace messages

}  // namespace spi