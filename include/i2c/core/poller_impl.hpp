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

template <typename Message>
concept ContinuousMessage =
    std::is_same_v<Message,
                   messages::ConfigureSingleRegisterContinuousPolling> ||
    std::is_same_v<Message, messages::ConfigureMultiRegisterContinuousPolling>;

template <typename Message>
concept LimitedMessage =
    std::is_same_v<Message, messages::SingleRegisterPollRead> ||
    std::is_same_v<Message, messages::MultiRegisterPollRead>;

template <template <class> class QueueImpl, timer::Timer TimerImpl,
          messages::I2CResponseQueue OwnQueueType>
requires MessageQueue<QueueImpl<writer::TaskMessage>, writer::TaskMessage>
struct ContinuousPoll {
    using I2CWriterType = writer::Writer<QueueImpl>;

    ContinuousPoll() = delete;
    ContinuousPoll(I2CWriterType& writer, OwnQueueType& own_queue)
        : timer(
              "i2c cts poller", [this]() -> void { do_next_transaction(); },
              100),
          own_queue(own_queue),
          writer(writer) {}
    ContinuousPoll(const ContinuousPoll&) = delete;
    auto operator=(const ContinuousPoll&) -> ContinuousPoll& = delete;
    ContinuousPoll(ContinuousPoll&&) = delete;
    auto operator=(ContinuousPoll&&) -> ContinuousPoll& = delete;
    ~ContinuousPoll() = default;
    std::array<messages::Transaction, 2> transactions{};
    decltype(transactions.cbegin()) current_transaction{transactions.cbegin()};
    messages::TransactionIdentifier id{};
    messages::ResponseWriter responder{};
    TimerImpl timer;
    OwnQueueType& own_queue;
    I2CWriterType& writer;

    template <ContinuousMessage Message>
    auto handle_message(Message& message) -> void {
        update_transactions(message);
        current_transaction = transactions.cbegin();
        id = message.id;
        responder = message.response_writer;
        timer.stop();
        if (message.delay_ms != 0) {
            LOG("Beginning or altering continuous poll of %#04x id %d @ %d ms",
                transactions[0].address, id.token, message.delay_ms);
            timer.update_period(message.delay_ms);
            timer.start();
        } else {
            transactions[0] = {};
            transactions[1] = {};
            id.token = 0;
        }
    }

    auto handle_response(messages::TransactionResponse& response) {
        response.id.is_completed_poll = false;
        static_cast<void>(responder.write(response));
        if (current_transaction != transactions.cbegin()) {
            do_next_transaction();
        }
    }

  private:
    auto update_transactions(
        const messages::ConfigureSingleRegisterContinuousPolling& msg) -> void {
        transactions[0] = msg.first;
        transactions[1] = {};
    }
    auto update_transactions(
        const messages::ConfigureMultiRegisterContinuousPolling& msg) -> void {
        transactions[0] = msg.first;
        transactions[1] = msg.second;
    }
    auto do_next_transaction() -> void {
        id.transaction_index =
            ((current_transaction == transactions.cbegin()) ? 0 : 1);
        writer.transact(*current_transaction, id, own_queue);
        if (current_transaction != transactions.cbegin()) {
            current_transaction = transactions.cbegin();
        } else if (transactions[1].address != 0) {
            current_transaction++;
        }
    }
};

template <template <class> class QueueImpl, timer::Timer TimerImpl,
          messages::I2CResponseQueue OwnQueueType>
requires MessageQueue<QueueImpl<writer::TaskMessage>, writer::TaskMessage>
struct LimitedPoll {
    using I2CWriterType = writer::Writer<QueueImpl>;
    LimitedPoll() = delete;
    LimitedPoll(I2CWriterType& writer, OwnQueueType& own_queue)
        : timer(
              "i2c limited poller", [this]() -> void { do_next_transaction(); },
              1),
          own_queue(own_queue),
          writer(writer) {}
    LimitedPoll(const LimitedPoll&) = delete;
    auto operator=(const LimitedPoll&) -> LimitedPoll& = delete;
    LimitedPoll(LimitedPoll&&) = delete;
    auto operator=(LimitedPoll&&) -> LimitedPoll& = delete;
    ~LimitedPoll() = default;
    std::array<messages::Transaction, 2> transactions{};
    decltype(transactions.cbegin()) current_transaction{transactions.cbegin()};
    int remaining_polls = 0;
    messages::TransactionIdentifier id{};
    messages::ResponseWriter responder{};
    TimerImpl timer;
    OwnQueueType& own_queue;
    I2CWriterType& writer;

    template <LimitedMessage Message>
    auto handle_message(Message& message) -> void {
        timer.stop();
        update_transactions(message);
        responder = message.response_writer;
        id = message.id;
        current_transaction = transactions.cbegin();
        if (message.delay_ms == 0 || message.polling == 0) {
            remaining_polls = 0;
            id.is_completed_poll = true;
            id.token = 0;
            LOG("stopping limited poll of %#04x because delay or polling is 0",
                transactions[0].address);
        } else {
            LOG("adding poll of %#04x @ %d ms for %d samples",
                transactions[0].address, message.delay_ms, remaining_polls);
            remaining_polls = message.polling;
            id.is_completed_poll = false;
            timer.update_period(message.delay_ms);
            timer.start();
        }
    }

    auto handle_response(messages::TransactionResponse& response) -> void {
        static_cast<void>(responder.write(response));
        if (response.id.is_completed_poll) {
            transactions[0] = {};
            transactions[1] = {};
            id.token = 0;
        } else {
            if (current_transaction != transactions.cbegin()) {
                do_next_transaction();
            }
        }
    }

  private:
    auto update_transactions(const messages::SingleRegisterPollRead& msg)
        -> void {
        transactions[0] = msg.first;
        transactions[1] = {};
    }
    auto update_transactions(const messages::MultiRegisterPollRead& msg)
        -> void {
        transactions[0] = msg.first;
        transactions[1] = msg.second;
    }

    auto do_next_transaction() -> void {
        const auto* first = transactions.cbegin();
        const auto* then_current = current_transaction;
        if (current_transaction != first) {
            remaining_polls--;
            current_transaction = first;
            id.transaction_index = 1;
        } else {
            if (transactions[1].address == 0) {
                remaining_polls--;
            } else {
                current_transaction++;
            }
            id.transaction_index = 0;
        }
        if (remaining_polls == 0) {
            timer.stop();
            id.is_completed_poll = true;
        }
        writer.transact(*then_current, id, own_queue);
    }
};

template <template <template <class> class, class, class> class PT,
          template <class> class QueueImpl, typename TimerImpl,
          typename OwnQueueType>
concept PollType =
    std::is_same_v<PT<QueueImpl, TimerImpl, OwnQueueType>,
                   ContinuousPoll<QueueImpl, TimerImpl, OwnQueueType>> ||
    std::is_same_v<PT<QueueImpl, TimerImpl, OwnQueueType>,
                   LimitedPoll<QueueImpl, TimerImpl, OwnQueueType>>;

template <template <template <class> class, class, class> class PollT,
          template <class> class QueueImpl, timer::Timer TimerImpl,
          messages::I2CResponseQueue OwnQueueType>
requires MessageQueue<QueueImpl<writer::TaskMessage>, writer::TaskMessage> &&
    PollType<PollT, QueueImpl, TimerImpl, OwnQueueType>

struct PollManager {
    static constexpr uint32_t MAX_CONTINUOUS_POLLS = 5;
    using PollType = PollT<QueueImpl, TimerImpl, OwnQueueType>;
    using Polls = std::array<PollType, MAX_CONTINUOUS_POLLS>;
    Polls polls;
    explicit PollManager(typename PollType::I2CWriterType& writer,
                         OwnQueueType& own_queue)
        : polls{PollType(writer, own_queue), PollType(writer, own_queue),
                PollType(writer, own_queue), PollType(writer, own_queue),
                PollType(writer, own_queue)} {}

    auto get_poller(messages::TransactionIdentifier id, bool& matched)
        -> PollType* {
        PollType* first_empty = nullptr;
        for (auto& poll : polls) {
            if (poll.id.token == id.token) {
                matched = true;
                return &poll;
            }
            if (!first_empty && poll.id.token == 0) {
                first_empty = &poll;
            }
        }
        matched = false;
        return first_empty;
    }

    template <typename Message>
    auto add_or_update(Message& message) -> void {
        bool matched = false;
        auto maybe_poller = get_poller(message.id, matched);
        if (!maybe_poller) {
            LOG("Could not add continuous poller for id %d",
                message.first.address, message.id.token);
            return;
        }
        maybe_poller->handle_message(message);
    }

    auto handle_response(messages::TransactionResponse& response) -> void {
        bool matched = false;
        auto maybe_poller = get_poller(response.id, matched);
        if (!matched) {
            return;
        }
        maybe_poller->handle_response(response);
    }
};

template <template <class> class QueueImpl, timer::Timer TimerImpl,
          messages::I2CResponseQueue OwnQueueType>
using LimitedPollManager =
    PollManager<LimitedPoll, QueueImpl, TimerImpl, OwnQueueType>;

template <template <class> class QueueImpl, timer::Timer TimerImpl,
          messages::I2CResponseQueue OwnQueueType>
using ContinuousPollManager =
    PollManager<ContinuousPoll, QueueImpl, TimerImpl, OwnQueueType>;

};  // namespace poller_impl
};  // namespace i2c
