#pragma once

#include "FreeRTOS.h"
#include "can_buffer_task.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "dispatch.hpp"

namespace freertos_can_dispatch {

template <message_buffer::MessageBuffer BufferType, can_message_buffer::CanMessageBufferListener Listener>
class FreeRTOSCanBufferPoller {
  public:
    FreeRTOSCanBufferPoller(BufferType & buffer, Listener & listener): buffer{buffer},listener{listener}, reader{buffer, listener} {
    }

    void operator() () {
        for (;;) {
            reader.read(portMAX_DELAY);
        }
    }

  private:
    BufferType & buffer;
    Listener & listener;
    can_message_buffer::CanMessageBufferReader<BufferType, Listener> reader;
};

}