/*
 * message_queue contains a concept defining a type-safe C++ template class
 * wrapping a thread-safe queue, which will be implemented in firmware as a
 * FreeRTOS queue and in tests as either a deque if the test does not use
 * threading, or something like a boost threadsafe queue if it does.
 */
#pragma once
#include <concepts>

template <typename Message1, typename Message2>
concept ConvertibleMessage = requires(const Message2& m2) {
    { Message1(m2) } -> std::same_as<Message1>;
};

template <class MQ, typename MessageType>
concept MessageQueue = requires(MQ mq, MessageType mt, const MQ cmq,
                                const MessageType cmt) {
    // Queues must have a try-send that takes a message instance by either
    // value, or const-ref, and may be callable with a second timeout argument.
    // If the timeout value is anything other than 0, it may block; whether or
    // not a timeout is specified, always check the return value.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    { mq.try_write(cmt, 10) } -> std::same_as<bool>;
    { mq.try_write(cmt) } -> std::same_as<bool>;
    { mq.try_write_isr(cmt) } -> std::same_as<bool>;
    // Queues must have a try-receive that takes a message instance by ptr for
    // filling in.
    { mq.try_read(&mt) } -> std::same_as<bool>;
    { mq.try_read_isr(&mt) } -> std::same_as<bool>;
    { mq.peek_isr(&mt) } -> std::same_as<bool>;
    { mq.peek(&mt, 10) } -> std::same_as<bool>;

    // Queues must have a const method to check whether there are messages.
    { cmq.has_message() } -> std::same_as<bool>;
    { cmq.has_message_isr() } -> std::same_as<bool>;
    // Defines the maximum wait time.
    { MQ::max_delay } -> std::convertible_to<int>;
};

template <class MQ, typename OtherMessageType>
concept RespondableMessageQueue = requires(MQ mq, const OtherMessageType& om) {
    { MQ::try_write_static(&mq, om) } -> std::same_as<bool>;
};
