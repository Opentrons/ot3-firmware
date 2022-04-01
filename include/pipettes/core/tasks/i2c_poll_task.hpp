#pragma once

#include "can/core/can_writer_task.hpp"
#include "common/core/i2c.hpp"
#include "common/core/timer.hpp"
#include "pipettes/core/i2c_poller.hpp"
#include "pipettes/core/i2c_poller_impl.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "pipettes/core/messages.hpp"

namespace i2c_poller_task {
using namespace i2c_poller_impl;

using TaskMessage = i2c_poller::TaskMessage;

template <template <class> class QueueImpl, timer::Timer TimerImpl>
requires MessageQueue<QueueImpl<i2c_writer::TaskMessage>,
                      i2c_writer::TaskMessage> &&
    MessageQueue<QueueImpl<i2c_poller::TaskMessage>, i2c_poller::TaskMessage>
class I2CPollingMessageHandler {
  public:
    using I2CWriterType = i2c_writer::I2CWriter<QueueImpl>;
    I2CPollingMessageHandler(I2CWriterType &i2c_writer)
        : i2c_writer{i2c_writer},
          continuous_polls(i2c_writer),
          limited_polls(i2c_writer) {}
    I2CPollingMessageHandler(const I2CPollingMessageHandler &) = delete;
    I2CPollingMessageHandler(const I2CPollingMessageHandler &&) = delete;
    auto operator=(const I2CPollingMessageHandler &)
        -> I2CPollingMessageHandler & = delete;
    auto operator=(const I2CPollingMessageHandler &&)
        -> I2CPollingMessageHandler && = delete;
    ~I2CPollingMessageHandler() = default;

    void handle_message(i2c_poller::TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(SingleRegisterPollReadFromI2C &m) {
        // TODO (lc, 03-01-2022): we should try to consolidate polling to
        // support any number of registers potentially.
        limited_polls.add(m);
    }

    void visit(MultiRegisterPollReadFromI2C &m) {
        // TODO (lc, 03-01-2022): we should try to consolidate polling to
        // support any number of registers potentially.
        limited_polls.add(m);
    }

    void visit(ConfigureSingleRegisterContinuousPolling &m) {
        continuous_polls.add_or_update(m);
    }

    void visit(ConfigureMultiRegisterContinuousPolling &m) {
        continuous_polls.add_or_update(m);
    }

    I2CWriterType &i2c_writer;

  public:
    // these are public to make testing easier
    ContinuousPollManager<QueueImpl, TimerImpl> continuous_polls;
    LimitedPollManager<QueueImpl, TimerImpl> limited_polls;

  private:
    // Default timeout should be 60 seconds
    // freertos expects this time to be in milliseconds
    static constexpr auto TIMEOUT = 60000;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl, timer::Timer TimerImpl>
requires MessageQueue<QueueImpl<i2c_poller::TaskMessage>,
                      i2c_poller::TaskMessage> &&
    MessageQueue<QueueImpl<i2c_writer::TaskMessage>, i2c_writer::TaskMessage>
class I2CPollingTask {
  public:
    using QueueType = QueueImpl<i2c_poller::TaskMessage>;
    using I2CWriterType = i2c_writer::I2CWriter<QueueImpl>;
    I2CPollingTask(QueueType &queue) : queue{queue} {}
    I2CPollingTask(const I2CPollingTask &c) = delete;
    I2CPollingTask(const I2CPollingTask &&c) = delete;
    auto operator=(const I2CPollingTask &c) = delete;
    auto operator=(const I2CPollingTask &&c) = delete;
    ~I2CPollingTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(I2CWriterType *writer) {
        auto handler = I2CPollingMessageHandler<QueueImpl, TimerImpl>{*writer};
        // Figure out task messages for I2C queue
        i2c_poller::TaskMessage message{};
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

}  // namespace i2c_poller_task
