#pragma once

#include "FreeRTOS.h"
#include "can_message_buffer.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "dispatch.hpp"
#include "message_core.hpp"

using namespace message_core;
using namespace message_buffer;
using namespace can_message_buffer;
using namespace can_dispatch;
using namespace freertos_message_buffer;

namespace freertos_can_dispatch {

/**
 * A FreeRTOS task entry point that polls a MessageBuffer.
 * @tparam BufferType The MessageBuffer type
 * @tparam Listener The CanMessageBufferListener type
 */
template <MessageBuffer BufferType,
          CanMessageBufferListener Listener>
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
    CanMessageBufferReader<BufferType, Listener> reader;
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
          CanMessage... MessageTypes>
requires HandlesMessages<HandlerType, MessageTypes...>
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

    using BufferType = FreeRTOMessageBuffer<BufferSize>;

    // The message buffer
    BufferType message_buffer;
    // A CanMessageBufferListener writing CAN messages into message_buffer.
    DispatchBufferTarget<BufferType, MessageTypes...> buffer_target;
    // A CanMessageBufferListener that parses CAN messages and notifies a
    // handler
    DispatchParseTarget<HandlerType, MessageTypes...> parse_target;
    // A freertos task entry point that polls message_buffer and notifies
    // parse_target.
    FreeRTOSCanBufferPoller<BufferType, DispatchParseTarget<HandlerType, MessageTypes...>> poller;
};

}  // namespace freertos_can_dispatch