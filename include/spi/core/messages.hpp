#pragma once

#include "common/core/message_queue.hpp"
#include "spi/core/utils.hpp"

namespace spi {

namespace messages {

/*
** Component struct for identifying a transaction. The primary identifier
** is an arbitrary uint8_t (typically the register id) - this should be set by
*the caller, and is
** available for setting in spi::writer::Writer
** index is which transaction just completed.
*/
struct TransactionIdentifier {
    uint8_t token;
    uint8_t command_type;
    bool send_response;

    auto operator==(const TransactionIdentifier& other) const -> bool = default;
};

struct Transaction {
    spi::utils::MaxMessageBuffer txBuffer;

    auto operator==(const Transaction&) const -> bool = default;
};

struct TransactResponse {
    auto operator==(const TransactResponse&) const -> bool = default;
    TransactionIdentifier id;
    spi::utils::MaxMessageBuffer rxBuffer;
    bool success;
};

/*
** A concept that can be used to identify a queue capable of receiving a
** Transaction Response.
*/
template <typename MessageQueue>
concept SpiResponseQueue =
    RespondableMessageQueue<MessageQueue, TransactResponse>;

/*
** Holds a special tiny little closure for writing response values
** that can be passed something with static lifetime - a normal
** function pointer. We need this because it will be memcpy'd, and
** so we can't use an actual safe closure like a std::function because
** it will get destroyed.
*/
struct ResponseWriter {
    template <SpiResponseQueue OriginatingQueue>
    explicit ResponseWriter(OriginatingQueue& rq)
        : queue_ref(reinterpret_cast<void*>(&rq)),
          writer(&OriginatingQueue::try_write_static) {}
    ResponseWriter() : queue_ref(nullptr), writer(nullptr) {}
    ResponseWriter(const ResponseWriter& other) = default;
    auto operator=(const ResponseWriter& other) -> ResponseWriter& = default;
    ResponseWriter(ResponseWriter&& other) = default;
    auto operator=(ResponseWriter&& other) -> ResponseWriter& = default;
    ~ResponseWriter() = default;
    void* queue_ref;
    bool (*writer)(void*, const TransactResponse&);

    auto write(const TransactResponse& response) -> bool {
        if (writer && queue_ref) {
            return writer(queue_ref, response);
        } else {
            return false;
        }
    }
};

struct Transact {
    TransactionIdentifier id;
    Transaction transaction;
    ResponseWriter response_writer;
};

}  // namespace messages

}  // namespace spi