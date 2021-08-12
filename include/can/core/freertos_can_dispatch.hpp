#pragma once

#include "FreeRTOS.h"
#include "can_message_buffer.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "dispatch.hpp"
#include "parse.hpp"

namespace freertos_can_dispatch {

template <message_buffer::MessageBuffer BufferType,
          can_message_buffer::CanMessageBufferListener Listener>
class FreeRTOSCanBufferPoller {
  public:
    FreeRTOSCanBufferPoller(BufferType& buffer, Listener& listener)
        : buffer{buffer}, listener{listener}, reader{buffer, listener} {}

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

template <std::size_t BufferSize, typename HandlerType,
          can_parse::CanMessage... MessageTypes>
requires can_dispatch::HandlesMessages<HandlerType, MessageTypes...>
struct FreeRTOSCanDispatcherTarget {
    FreeRTOSCanDispatcherTarget(HandlerType& handler)
        : message_buffer{},
          buffer_target{message_buffer},
          parse_target{handler},
          poller{message_buffer, parse_target} {}

    using BufferType =
        freertos_message_buffer::FreeRTOMessageBuffer<BufferSize>;
    BufferType message_buffer;
    can_dispatch::DispatchBufferTarget<BufferType, MessageTypes...>
        buffer_target;
    can_dispatch::DispatchParseTarget<HandlerType, MessageTypes...>
        parse_target;
    FreeRTOSCanBufferPoller<BufferType, can_dispatch::DispatchParseTarget<
                                            HandlerType, MessageTypes...>>
        poller;
};

}  // namespace freertos_can_dispatch