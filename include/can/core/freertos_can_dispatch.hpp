#pragma once

#include "FreeRTOS.h"
#include "can_bus.hpp"
#include "can_message_buffer.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "common/core/freertos_task.hpp"
#include "dispatch.hpp"
#include "message_core.hpp"

namespace freertos_can_dispatch {

using namespace message_core;
using namespace message_buffer;
using namespace can_bus;
using namespace can_message_buffer;
using namespace can_dispatch;
using namespace freertos_message_buffer;
using namespace freertos_task;

/**
 * A FreeRTOS task entry point that polls a MessageBuffer.
 * @tparam BufferType The MessageBuffer type
 * @tparam Listener The CanMessageBufferListener type
 */
template <MessageBuffer BufferType, CanMessageBufferListener Listener>
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
    [[noreturn]] void operator()() {
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
 * A helper to build a FreeRTOSMessageBuffer holding can messages to build
 * supporting classes to poll and parse the messages.
 *
 * @tparam BufferSize The size of the FreeRTOSMessageBuffer in bytes
 * @tparam HandlerType The type of the parsed message callback. Must conform to
 * HandlesMessages concept.
 * @tparam StackDepth Stack size of polling FreeRTOS task
 * @tparam Priority Priority of polling FreeRTOS task
 * @tparam MessageTypes The CanMessage types this MessageBuffer is interested
 * in.
 */
template <std::size_t BufferSize, typename HandlerType, uint32_t StackDepth,
          UBaseType_t Priority, CanMessage... MessageTypes>
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
          poller{message_buffer, parse_target},
          task{poller} {}

    using BufferType = FreeRTOSMessageBuffer<BufferSize>;

    // The message buffer
    BufferType message_buffer;
    // A CanMessageBufferListener writing CAN messages into message_buffer.
    DispatchBufferTarget<BufferType, MessageTypes...> buffer_target;
    // A CanMessageBufferListener that parses CAN messages and notifies a
    // handler
    DispatchParseTarget<HandlerType, MessageTypes...> parse_target;
    // A freertos task entry point that polls message_buffer and notifies
    // parse_target.
    FreeRTOSCanBufferPoller<BufferType,
                            DispatchParseTarget<HandlerType, MessageTypes...>>
        poller;
    FreeRTOSTask<StackDepth, Priority> task;
};

/**
 * A FreeRTOS task entry point that registers a callback with CAN, writes
 * messages to a MessageBuffer and dispatches new messages to Dispatcher.
 *
 * @tparam BufferSize The message buffer size
 * @tparam Dispatcher The dispatcher to receive CAN messages.
 */
template <std::size_t BufferSize, CanMessageBufferListener Dispatcher>
class FreeRTOSCanReader {
  public:
    /**
     * Constructor
     */
    FreeRTOSCanReader(CanBus& can_bus, Dispatcher& dispatcher)
        : can_bus{can_bus},
          dispatcher{dispatcher},
          message_buffer{},
          buffer_writer{message_buffer},
          buffer_reader{message_buffer, dispatcher} {}

    /**
     * The task entry
     */
    [[noreturn]] void operator()() {
        can_bus.set_incoming_message_callback(
            this, FreeRTOSCanReader<BufferSize, Dispatcher>::callback);
        for (;;) {
            buffer_reader.read(portMAX_DELAY);
        }
    }

  private:
    /**
     * CAN ISR callback
     * @param instance_data
     * @param identifier
     * @param data
     * @param length
     */
    static void callback(void* instance_data, uint32_t identifier,
                         uint8_t* data, uint8_t length) {
        auto instance = static_cast<FreeRTOSCanReader<BufferSize, Dispatcher>*>(
            instance_data);
        instance->buffer_writer.send_from_isr(identifier, data, data + length);   // NOLINT
    }

    using BufferType = FreeRTOSMessageBuffer<BufferSize>;
    CanBus& can_bus;
    Dispatcher& dispatcher;
    BufferType message_buffer;
    CanMessageBufferWriter<BufferType> buffer_writer;
    CanMessageBufferReader<BufferType, Dispatcher> buffer_reader;
};

}  // namespace freertos_can_dispatch