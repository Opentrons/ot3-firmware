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
          task_control{},
          task{poller, task_control, 5, ""} {}

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
    FreeRTOSTaskControl<StackDepth> task_control;
    FreeRTOSTask<
        StackDepth,
        FreeRTOSCanBufferPoller<
            BufferType, DispatchParseTarget<HandlerType, MessageTypes...>>>
        task;
};

/**
 * A FreeRTOS message buffer for CAN messages along with a reader and writer.
 * @tparam BufferSize Size of the buffer in bytes.
 * @tparam Dispatcher Dispatcher that will receive messages.
 */
template <std::size_t BufferSize, CanMessageBufferListener Dispatcher>
struct FreeRTOSCanBufferControl {
    FreeRTOSCanBufferControl(Dispatcher& dispatcher)
        : message_buffer{},
          writer{message_buffer},
          reader{message_buffer, dispatcher} {}

    using BufferType = FreeRTOSMessageBuffer<BufferSize>;
    BufferType message_buffer;
    CanMessageBufferWriter<BufferType> writer;
    CanMessageBufferReader<BufferType, Dispatcher> reader;
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
    using Control = FreeRTOSCanBufferControl<BufferSize, Dispatcher>;
    /**
     * Constructor
     */
    FreeRTOSCanReader(CanBus& can_bus, Control& message_buffer)
        : can_bus(can_bus), message_buffer{message_buffer} {}

    /**
     * The task entry
     */
    [[noreturn]] void operator()() {
        can_bus.set_incoming_message_callback(
            this, FreeRTOSCanReader<BufferSize, Dispatcher>::callback);
        for (;;) {
            message_buffer.reader.read(portMAX_DELAY);
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
        instance->message_buffer.writer.send_from_isr(identifier, data,
                                                      data + length);  // NOLINT
    }

    CanBus& can_bus;
    Control& message_buffer;
};

}  // namespace freertos_can_dispatch