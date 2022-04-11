#pragma once

#include "common/core/freertos_synchronization.hpp"

namespace spi {

namespace messages {

using std::size_t;
static constexpr std::size_t MAX_BUFFER_SIZE = 5;
using MaxMessageBuffer = std::array<uint8_t, MAX_BUFFER_SIZE>;

/*
** Component struct for identifying a transaction. The primary identifier
** is an arbitrary uint32_t - this should be set by the caller, and is
** available for setting in i2c::writer::Writer - and an add on bool used
** for chaining response messages through the poller.
** The transaction index is used for multi-register reads; for these, the
** index is which transaction just completed.
*/
struct TransactionIdentifier {
    uint32_t token;
    uint8_t transaction_index;

    auto operator==(const TransactionIdentifier& other) const -> bool = default;
};

struct Transaction = {
    MaxMessageBuffer txData;
    MaxMessageBuffer rxData;

    auto operator==(const Transaction&) const -> bool = default;
};

struct TransactionResponse = {
    TransactionIdentifier id;
    size_t bytes_read;
    MaxMessageBuffer read_buffer;

    auto operator==(const TransactionResponse&) const -> bool = default;
};

/*
** A concept that can be used to identify a queue capable of receiving a
** Transaction Response.
*/
template <typename MessageQueue>
concept SpiResponseQueue =
RespondableMessageQueue<MessageQueue, TransactionResponse>;

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
    bool (*writer)(void*, const TransactionResponse&);

    auto write(const TransactionResponse& response) -> bool {
        if (writer && queue_ref) {
            return writer(queue_ref, response);
        } else {
            return false;
        }
    }
};

} // namespace messages

} // namespace spi