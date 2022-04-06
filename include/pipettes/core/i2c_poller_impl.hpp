#pragma once

#include <array>
#include <cstdint>
#include <type_traits>
#include <variant>

#include "common/core/logging.h"
#include "common/core/timer.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "pipettes/core/messages.hpp"
#include "sensors/core/callback_types.hpp"

namespace i2c_poller_impl {
using namespace pipette_messages;
using namespace sensor_callbacks;

template <typename Message>
concept ContinuousPollMessage =
    std::is_same_v<Message, ConfigureSingleRegisterContinuousPolling> ||
    std::is_same_v<Message, ConfigureMultiRegisterContinuousPolling>;

template <typename Message>
concept LimitedPollMessage =
    std::is_same_v<Message, SingleRegisterPollReadFromI2C> ||
    std::is_same_v<Message, MultiRegisterPollReadFromI2C>;

template <template <class> class QueueImpl, timer::Timer TimerImpl>
requires MessageQueue<QueueImpl<i2c_writer::TaskMessage>,
                      i2c_writer::TaskMessage>
struct ContinuousPoll {
    using I2CWriterType = i2c_writer::I2CWriter<QueueImpl>;
    ContinuousPoll(I2CWriterType& writer)
        : address(0),
          timer(
              "i2c cts poller", []() {}, 100),
          writer(writer) {}
    uint16_t address;
    TimerImpl timer;
    I2CWriterType& writer;
    uint32_t poll_id = 0;

    template <ContinuousPollMessage Message>
    auto handle_message(const Message& message) -> void {
        address = message.address;
        poll_id = message.poll_id;
        timer.stop();
        if (message.delay_ms != 0) {
            LOG("Beginning or altering continuous poll of %#04x id %d @ %d ms",
                address, poll_id, message.delay_ms);
            timer.update_callback(provide_callback(message));
            timer.update_period(message.delay_ms);
            timer.start();
        } else {
            timer.stop();
            address = 0;
            poll_id = 0;
        }
    }

  private:
    auto provide_callback(
        const ConfigureSingleRegisterContinuousPolling& message)
        -> std::function<void(void)> {
        return [this, message]() {
            this->writer.transact(
                this->address, message.buffer,
                []() {},  // nothing for the client callback - we
                // only handle individual samples
                message.handle_buffer);
        };
    }

    auto provide_callback(
        const ConfigureMultiRegisterContinuousPolling& message)
        -> std::function<void(void)> {
        return [this, message]() {
            this->writer.transact(
                this->address, message.register_1_buffer, []() {},
                [this, message](const MaxMessageBuffer& buffer) {
                    auto first_buf = buffer;
                    this->writer.transact(
                        this->address, message.register_2_buffer, []() {},
                        [first_buf,
                         message](const MaxMessageBuffer& second_buf) {
                            message.handle_buffer(first_buf, second_buf);
                        });
                });
        };
    }
};

template <template <class> class QueueImpl, timer::Timer TimerImpl>
requires MessageQueue<QueueImpl<i2c_writer::TaskMessage>,
                      i2c_writer::TaskMessage>
struct LimitedPoll {
    using I2CWriterType = i2c_writer::I2CWriter<QueueImpl>;
    explicit LimitedPoll(I2CWriterType& writer)
        : address(0),
          timer(
              "i2c limited poller", []() {}, 1),
          writer(writer) {}
    uint16_t address;
    TimerImpl timer;
    I2CWriterType& writer;

    template <LimitedPollMessage Message>
    auto handle_message(Message& message) -> void {
        timer.stop();
        if (message.delay_ms != 0 || message.polling == 0) {
            LOG("adding poll of %#04x @ %d ms for %d samples", message.address,
                message.delay_ms, message.polling);
            address = message.address;
            timer.update_callback(provide_callback(message));
            timer.update_period(message.delay_ms);
            timer.start();
        } else {
            address = 0;
            LOG("stopping limited poll of %#04x because delay is 0",
                message.address);
        }
    }

  private:
    static auto do_recursive_cb_single(
        LimitedPoll<QueueImpl, TimerImpl>* slf,
        const SingleRegisterPollReadFromI2C& message,
        const MaxMessageBuffer& buffer) -> void {
        auto local_message = message;
        local_message.handle_buffer(buffer);
        local_message.polling--;
        if (local_message.polling == 0) {
            // base case: no more polls, call the final callback
            local_message.client_callback();
            slf->timer.stop();
            slf->address = 0;
            LOG("stopping limited poll of %$04x because all samples have been "
                "taken",
                message.address);
        } else {
            // recurse to change the timer callback to the new message with the
            // new polling
            slf->timer.update_callback(slf->provide_callback(local_message));
        }
    }
    auto provide_callback(const SingleRegisterPollReadFromI2C& message)
        -> std::function<void(void)> {
        return ([this, message]() {
            this->writer.transact(
                this->address, message.buffer, []() {},
                [this, message](const MaxMessageBuffer& buf) {
                    do_recursive_cb_single(this, message, buf);
                });
        });
    }

    static auto do_recursive_cb_multi(
        LimitedPoll<QueueImpl, TimerImpl>* slf,
        const MultiRegisterPollReadFromI2C& message,
        const MaxMessageBuffer& first_buffer,
        const MaxMessageBuffer& second_buffer) -> void {
        auto local_message = message;
        local_message.polling--;
        local_message.handle_buffer(first_buffer, second_buffer);
        if (local_message.polling == 0) {
            // base case
            local_message.client_callback();
            slf->timer.stop();
            slf->address = 0;
            LOG("stopping limited poll of %$04x because all samples have been "
                "taken",
                message.address);

        } else {
            slf->timer.update_callback(slf->provide_callback(local_message));
        }
    }
    auto provide_callback(MultiRegisterPollReadFromI2C message)
        -> std::function<void(void)> {
        LOG("providing multi callback with this=%p", this);
        auto second_stage = std::function<void(const MaxMessageBuffer&)>([=, this](const MaxMessageBuffer& first_buffer) {
            LOG("second stage mrprfi2c callback NOW with this=%p", this);
            writer.transact(
                address, message.register_2_buffer, []() {},
                [=, this](const MaxMessageBuffer& second_buffer) {
                    LOG("third state mrprfi2c callback NOW with this=%p", this);
                    do_recursive_cb_multi(this, message, first_buffer,
                                          second_buffer);
                });
        });
        return [=, this]() {
            LOG("first stage mrprfi2c callback NOW with this=%p", this);
            writer.transact(
                address, message.register_1_buffer, []() {},
                second_stage);
        };
    }
};

template <template <class> class QueueImpl, timer::Timer TimerImpl>
requires MessageQueue<QueueImpl<i2c_writer::TaskMessage>,
                      i2c_writer::TaskMessage>
struct ContinuousPollManager {
    static constexpr uint32_t MAX_CONTINUOUS_POLLS = 5;
    using PollType = ContinuousPoll<QueueImpl, TimerImpl>;
    using Polls = std::array<PollType, MAX_CONTINUOUS_POLLS>;
    Polls polls;
    explicit ContinuousPollManager(PollType::I2CWriterType& writer)
        : polls{PollType(writer), PollType(writer), PollType(writer),
                PollType(writer), PollType(writer)} {}

    auto get_poller(uint16_t address, uint32_t id) -> PollType* {
        PollType* first_empty = nullptr;
        for (auto& poll : polls) {
            if ((poll.address == address) && (poll.poll_id == id)) {
                return &poll;
            }
            if (poll.address == 0 && !first_empty) {
                first_empty = &poll;
            }
        }
        return first_empty;
    }

    template <ContinuousPollMessage Message>
    auto add_or_update(Message& message) -> void {
        auto maybe_poller = get_poller(message.address, message.poll_id);
        if (!maybe_poller) {
            LOG("Could not add continuous poller for address %#04x id %d",
                message.address, message.poll_id);
            return;
        } else {
            maybe_poller->handle_message(message);
        }
    }
};

template <template <class> class QueueImpl, timer::Timer TimerImpl>
requires MessageQueue<QueueImpl<i2c_writer::TaskMessage>,
                      i2c_writer::TaskMessage>
struct LimitedPollManager {
    static constexpr uint32_t MAX_LIMITED_POLLS = 5;
    using PollType = LimitedPoll<QueueImpl, TimerImpl>;
    using Polls = std::array<PollType, MAX_LIMITED_POLLS>;
    Polls polls{};
    explicit LimitedPollManager(PollType::I2CWriterType& writer)
        : polls{PollType(writer), PollType(writer), PollType(writer),
                PollType(writer), PollType(writer)} {}

    auto get_poll_slot() -> PollType* {
        for (auto& poll : polls) {
            if (!poll.timer.is_running()) {
                return &poll;
            }
        }
        return nullptr;
    }
    template <LimitedPollMessage Message>
    auto add(Message& message) -> void {
        auto* maybe_poller = get_poll_slot();
        if (!maybe_poller) {
            LOG("Could not add limited poller for addr %#04x @ %d ms (no "
                "slots)",
                message.address, message.delay_ms);
            return;
        } else {
            maybe_poller->handle_message(message);
        }
    }
};
};  // namespace i2c_poller_impl
