#pragma once

#include "FreeRTOS.h"
#include "can_message_buffer.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "dispatch.hpp"
#include "parse.hpp"

namespace freertos_can_dispatch {

/**
 * A FreeRTOS task entry point that polls a MessageBuffer.
 * @tparam BufferType The MessageBuffer type
 * @tparam Listener The CanMessageBufferListener type
 */
template <message_buffer::MessageBuffer BufferType,
          can_message_buffer::CanMessageBufferListener Listener>
class FreeRTOSCanBufferPoller {
  public:
    /**
     * Constructor
     * @param buffer The MessageBuffer instance (ie FreeRTOSMessageBuffer)
     * @param listener The listener to be called back when a new message is in
     * the buffer.
     */
    FreeRTOSCanBufferPoller(BufferType& buffer, Listener& listener)
        : buffer{buffer}, listener{listener}, reader{buffer, listener} {}

    /**
     * Task entry point.
     */
    void operator()() {
        for (;;) {
            reader.read(portMAX_DELAY);
        }
    }

  private:
    BufferType& buffer;
    Listener& listener;
    can_message_buffer::CanMessageBufferReader<BufferType, Listener> reader;
};

/**
 * A helper to build a FreeRTOMessageBuffer holding can messages to build
 * supporting classes to poll and parse the messages.
 *
 * @tparam BufferSize The size of the FreeRTOMessageBuffer in bytes
 * @tparam HandlerType The type of the parsed message callback. Must conform to
 * HandlesMessages concept.
 * @tparam MessageTypes The CanMessage types this MessageBuffer is interested
 * in.
 */
template <std::size_t BufferSize, typename HandlerType,
          can_parse::CanMessage... MessageTypes>
requires can_dispatch::HandlesMessages<HandlerType, MessageTypes...>
struct FreeRTOSCanDispatcherTarget {
    /**
     * Constructor.
     *
     * @param handler The class called with a fully parsed message in the
     * message buffer.
     */
    FreeRTOSCanDispatcherTarget(HandlerType& handler)
        : message_buffer{},
          buffer_target{message_buffer},
          parse_target{handler},
          poller{message_buffer, parse_target} {}

    using BufferType =
        freertos_message_buffer::FreeRTOMessageBuffer<BufferSize>;

    // The message buffer
    BufferType message_buffer;
    // A CanMessageBufferListener writing CAN messages into message_buffer.
    can_dispatch::DispatchBufferTarget<BufferType, MessageTypes...>
        buffer_target;
    // A CanMessageBufferListener that parses CAN messages and notifies a
    // handler
    can_dispatch::DispatchParseTarget<HandlerType, MessageTypes...>
        parse_target;
    // A freertos task entry point that polls message_buffer and notifies
    // parse_target.
    FreeRTOSCanBufferPoller<BufferType, can_dispatch::DispatchParseTarget<
                                            HandlerType, MessageTypes...>>
        poller;
};

}  // namespace freertos_can_dispatch