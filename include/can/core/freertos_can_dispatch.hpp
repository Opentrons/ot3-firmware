#pragma once

#include "FreeRTOS.h"
#include "can_bus.hpp"
#include "can_message_buffer.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "common/core/freertos_task.hpp"
#include "dispatch.hpp"
#include "message_core.hpp"

namespace can::freertos_dispatch {

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
            reader.read(buffer.max_delay);
        }
    }

  private:
    BufferType& buffer;
    Listener& listener;
    CanMessageBufferReader<BufferType, Listener> reader;
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
    FreeRTOSCanReader(Control& message_buffer)
        : message_buffer{message_buffer} {}

    /**
     * The task entry
     */
    [[noreturn]] void operator()(CanBus* can_bus) {
        can_bus->set_incoming_message_callback(
            this, FreeRTOSCanReader<BufferSize, Dispatcher>::callback);
        for (;;) {
            message_buffer.reader.read(message_buffer.message_buffer.max_delay);
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

    Control& message_buffer;
};

}  // namespace freertos_can_dispatch