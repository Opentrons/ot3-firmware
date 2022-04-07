#pragma once

#include <array>
#include <cstdint>
#include <type_traits>
#include <variant>

#include "common/core/logging.h"
#include "common/core/timer.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/writer.hpp"

namespace i2c {
namespace poller_impl {
using namespace messages;

template <template <class> class QueueImpl, timer::Timer TimerImpl,
          I2CResponseQueue OwnResponderQueue>
requires MessageQueue<QueueImpl<i2c_writer::TaskMessage>,
                      i2c_writer::TaskMessage>
struct ContinuousPoll {
    using I2CWriterType = i2c_writer::I2CWriter<QueueImpl>;

    template <typename Message>
    concept OwnMessage =
        std::is_same_v<Message, ConfigureSingleRegisterContinuousPolling> ||
        std::is_same_v<Message, ConfigureMultiRegisterContinuousPolling>;

    ContinuousPoll(I2CWriterType& writer)
        : transactions{{0, 0, 0, {}}, {0, 0, 0, {}}},
          next_transaction(transactions.cbegin()),
          id{.token = 0, .is_completed_poll = false},
          timer(
              "i2c cts poller", [this]() -> void { do_next_transaction(); },
              100),
          writer(writer) {}
    std::array<Transaction, 2> transactions;
    decltype(transactions.cbegin()) current_transaction;
    TransactionIdentifier id;
    OwnResponderQueue& own_queue;

    TimerImpl timer;
    I2CWriterType& writer;

    template <OwnMessage Message>
    auto handle_message(const Message& message) -> void {
        poll_id = message.poll_id;
        timer.stop();
        update_own_txns(message);
        if (message.delay_ms != 0) {
            LOG("Beginning or altering continuous poll of %#04x id %d @ %d ms",
                transactions[0].address, poll_id, message.delay_ms);
            current_transaction = transactions.cbegin();
            timer.update_period(message.delay_ms);
            timer.start();
        } else {
            timer.stop();
            poll_id = 0;
        }
    }

  private:
    void do_next_transaction() {
        writer.transact(*current_transaction, id, own_queue);
    }
    void update_own_txns(const ConfigureSingleRegisterContinuousPolling& msg) {
        if (msg.delay_ms) {
            transactions[0] = msg.transaction;
        } else {
            transactions[0].address = 0;
        }
        transactions[1].address = 0;
    }
    void update_own_txn(const ConfigureMultiRegisterContinuousPolling& msg) {
        if (msg.delay_ms) {
            transactions[0] = msg.first;
            transactions[1] = msg.second;
        } else {
            transactions[0].address = 0;
            transactions[1].address = 0;
        }
    }

    auto get_next_transaction() -> decltype(next_transaction) {
        if (current_transaction == transactions.cbegin() &&
            transactions[1].address) {
            // this was the first transaction of a multi-transaction poll
            return current_transaction + 1;
        } else if (current_transaction == (transactions.cbegin()++)) {
            // this was the last transaction of a multi-transaction poll
            return transactions.cbegin();
        } else {
            // this was the only transaction of a one-transaction poll
            return current_transaction;
        }
    }
};

template <template <class> class QueueImpl, timer::Timer TimerImpl,
          I2CResponseQueue OwnResponderQueue>
requires MessageQueue<QueueImpl<i2c_writer::TaskMessage>,
                      i2c_writer::TaskMessage>
struct LimitedPoll {
    template <typename Message>
    concept OwnMessage =
        std::is_same_v<Message, SingleRegisterPollReadFromI2C> ||
        std::is_same_v<Message, MultiRegisterPollReadFromI2C>;

    using I2CWriterType = i2c_writer::I2CWriter<QueueImpl>;
    LimitedPoll(I2CWriterType& writer, OwnResponderQueue& own_queue)
        : transactions{{0, 0, 0, {}}, {0, 0, 0, {}}},
          next_transaction(transactions.cbegin()),
          id{.token = 0, .is_completed_poll = false},
          remaining_polls(0),
          timer(
              "i2c limited poller", [this]() -> void { do_next_transaction(); },
              1),
          writer(writer),
          own_queue(own_queue) {}
    std::array<Transaction, 2> transactions;
    decltype(transactions.cbegin()) current_transaction;
    TransactionIdentifier id;
    std::size_t remaining_polls;
    ResponseWriter response;
    TimerImpl timer;
    I2CWriterType& writer;
    OwnResponderQueue& own_queue;

    template <OwnMessage Message>
    auto handle_message(const Message& message) -> void {
        timer.stop();
        if (message.delay_ms == 0 || message.polling == 0) {
            LOG("stopping limited poll because delay is 0",
                transactions[0].address);
            remaining_polls = 0;
            id.token = 0;
        }
        LOG("adding poll of %#04x @ %d ms for %d samples",
            transactions[0].address, message.delay_ms, message.polling);
        update_own_txn(message);
        next_transaction = transactions.cbegin();
        id = message.id;
        timer.update_period(message.delay_ms);
        timer.start();
    }

    auto handle_response(const TransactionResponse& m) -> void {
        response.write(m);
    }

  private:
    auto get_next_transaction() -> decltype(next_transaction) {
        if (current_transaction == transactions.cbegin() &&
            transactions[1].address) {
            // this was the first transaction of a multi-transaction poll
            return current_transaction + 1;
        } else if (current_transaction == (transactions.cbegin()++)) {
            // this was the last transaction of a multi-transaction poll
            return transactions.cbegin();
        } else {
            // this was the only transaction of a one-transaction poll
            return current_transaction;
        }
    }

    auto get_polls_left_after_this() -> std::size_t {
        auto next = get_next_transaction();
        if (next == current) {
            // this is a one-transaction poll
            return remaining_polls - 1;
        } else if (next < current) {
            // we are about to finish the second transaction
            return remaining_polls - 1;
        } else {
            // we have a transaction left
            return remaining_polls;
        }
    }

    void do_next_transaction() {
        if (remaining_polls = get_polls_left_after_this() == 0) {
            id.is_completed_poll = true;
        }
        writer.transact(*current_transaction, id, own_queue);
        current_transaction = get_next_transaction();
        if (remaining_polls == 0) {
            timer.stop();
        }
    }

    void update_own_txns(const SingleRegisterPollRead& msg) {
        transactions[0] = msg.first;
        transactions[1].address = 0;
    }
    void update_own_txn(const MultiRegisterPollRead& msg) {
        transactions[0] = msg.first;
        transactions[1] = msg.second;
    }
};

template <typename<class> class WhichPoller, template <class> class QueueImpl,
          timer::Timer TimerImpl, I2CResponseQueue OwnQueue>
concept PollType = std::is_same_v<
    WhichPoller<QueueImpl<writer::TaskMessage>, TimerImpl, OwnQueue>,
    ContinuousPoll<QueueImpl<writer::TaskMessage>, TimerImpl, OwnQueue>> ||
    std::is_same_v<
        WhichPoller<QueueImpl<writer::TaskMessage>, TimerImpl, OwnQueue>,
        LimitedPoll<QueueImpl<writer::TaskMessage>, TimerImpl, OwnQueue>>;

template <template <class> class QueueImpl, timer::Timer TimerImpl,
          I2CResponseQueue OwnQueue, template <class> class Poll>
requires MessageQueue<QueueImpl<writer::TaskMessage>, writer::TaskMessage> &&
    PollType<Poll, QueueImpl, TimerImpl, OwnQueue>
struct PollManager {
    static constexpr uint32_t MAX_POLLS = 5;
    using PollType = Poll<QueueImpl, TimerImpl>;
    using Polls = std::array<PollType, MAX_POLLS>;
    Polls polls;
    explicit PollManager(PollType::I2CWriterType& writer, OwnQueue& own_queue)
        : polls{PollType(writer, own_queue), PollType(writer, own_queue),
                PollType(writer, own_queue), PollType(writer, own_queue),
                PollType(writer, own_queue)} {}

    auto get_poller(Transaction id, or_empty = true) -> PollType* {
        PollType* first_empty = nullptr;
        for (auto& poll : polls) {
            if (id == poll.id) {
                return &poll;
            }
            if (!first_empty) {
                first_empty = &poll;
            }
        }
        return or_empty ? first_empty : nullptr;
    }

    template <PollType::OwnMessage Message>
    auto add_or_update(const Message& message) -> void {
        auto maybe_poller = get_poller(message.id);
        if (!maybe_poller) {
            LOG("Could not add poller for address %#04x id %d",
                message.first.address, message.poll_id);
            return;
        } else {
            maybe_poller->handle_message(message);
        }
    }

    auto handle_response(const TransactionResponse& message) -> void {
        auto maybe_poller = get_poller(message.id, false);
        if (maybe_poller) {
            maybe_poller->handle_response(message);
        }
    }
};

template <template <class> class QueueImpl, timer::Timer TimerImpl,
          I2CResponseQueue OwnQueue>
using ContinuousPollManager =
    PollManager<QueueImpl, TimerImpl, OwnQueue, ContinuousPoll>;

template <template <class> class QueueImpl, timer::Timer TimerImpl,
          I2CResponseQueue OwnQueue>
using LimitedPollManager =
    PollManager<QueueImpl, TimerImpl, OwnQueue, LimitedPoll>;

};  // namespace poller_impl
};  // namespace i2c
