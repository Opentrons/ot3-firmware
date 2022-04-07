#pragma once

#include "can/core/can_writer_task.hpp"
#include "common/core/timer.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/poller.hpp"
#include "i2c/core/poller_impl.hpp"
#include "i2c/core/writer.hpp"

namespace i2c {
namespace tasks {

template <template <class> class QueueImpl, timer::Timer TimerImpl,
          messages::I2CResponseQueue OwnQueueType>
requires MessageQueue<QueueImpl<writer::TaskMessage>, writer::TaskMessage> &&
    MessageQueue<QueueImpl<poller::TaskMessage>, poller::TaskMessage>
class I2CPollerMessageHandler {
  public:
    using I2CWriterType = writer::Writer<QueueImpl>;
    I2CPollerMessageHandler(I2CWriterType &i2c_writer, OwnQueueType &own_queue)
        : i2c_writer{i2c_writer},
          own_queue(own_queue),
          continuous_polls(i2c_writer, own_queue),
          limited_polls(i2c_writer, own_queue) {}
    I2CPollerMessageHandler(const I2CPollerMessageHandler &) = delete;
    I2CPollerMessageHandler(const I2CPollerMessageHandler &&) = delete;
    auto operator=(const I2CPollerMessageHandler &)
        -> I2CPollerMessageHandler & = delete;
    auto operator=(const I2CPollerMessageHandler &&)
        -> I2CPollerMessageHandler && = delete;
    ~I2CPollerMessageHandler() = default;

    void handle_message(poller::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(messages::SingleRegisterPollRead &m) {
        // TODO (lc, 03-01-2022): we should try to consolidate polling to
        // support any number of registers potentially.
        limited_polls.add_or_update(m);
    }

    void visit(messages::MultiRegisterPollRead &m) {
        // TODO (lc, 03-01-2022): we should try to consolidate polling to
        // support any number of registers potentially.
        limited_polls.add_or_update(m);
    }

    void visit(messages::ConfigureSingleRegisterContinuousPolling &m) {
        continuous_polls.add_or_update(m);
    }

    void visit(messages::ConfigureMultiRegisterContinuousPolling &m) {
        continuous_polls.add_or_update(m);
    }

    void visit(messages::TransactionResponse &m) {
        limited_polls.handle_response(m);
        continuous_polls.handle_response(m);
    }

    I2CWriterType &i2c_writer;
    OwnQueueType &own_queue;

  public:
    // these are public to make testing easier

    poller_impl::ContinuousPollManager<QueueImpl, TimerImpl, OwnQueueType>
        continuous_polls;
    poller_impl::LimitedPollManager<QueueImpl, TimerImpl, OwnQueueType>
        limited_polls;

  private:
    // Default timeout should be 60 seconds
    // freertos expects this time to be in milliseconds
    static constexpr auto TIMEOUT = 60000;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl, timer::Timer TimerImpl>
requires MessageQueue<QueueImpl<poller::TaskMessage>, poller::TaskMessage> &&
    MessageQueue<QueueImpl<writer::TaskMessage>, writer::TaskMessage>
class I2CPollerTask {
  public:
    using QueueType = QueueImpl<poller::TaskMessage>;
    using I2CWriterType = i2c::writer::Writer<QueueImpl>;
    I2CPollerTask(QueueType &queue) : queue{queue} {}
    I2CPollerTask(const I2CPollerTask &c) = delete;
    I2CPollerTask(const I2CPollerTask &&c) = delete;
    auto operator=(const I2CPollerTask &c) = delete;
    auto operator=(const I2CPollerTask &&c) = delete;
    ~I2CPollerTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(I2CWriterType *writer) {
        auto handler = I2CPollerMessageHandler<QueueImpl, TimerImpl, QueueType>{
            *writer, get_queue()};
        // Figure out task messages for I2C queue
        poller::TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
};
};  // namespace tasks
}  // namespace i2c
