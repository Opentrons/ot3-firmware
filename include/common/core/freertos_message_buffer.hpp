#pragma once

#include <cstdint>

#include "message_buffer.h"

namespace freertos_message_buffer {

template <std::size_t BufferSize>
class FreeRTOMessageBuffer {
  public:
    explicit FreeRTOMessageBuffer();
    ~FreeRTOMessageBuffer();

    auto send(const uint8_t* buffer, std::size_t buffer_length,
              uint32_t timeout) -> std::size_t;
    auto send_from_isr(const uint8_t* buffer, std::size_t buffer_length)
        -> std::size_t;
    auto receive(uint8_t* buffer, std::size_t buffer_length, uint32_t timeout)
        -> std::size_t;

  private:
    StaticMessageBuffer_t message_buffer;
    MessageBufferHandle_t handle;
    Listener& listener;
    std::array<uint8_t, BufferSize> backing;
};

FreeRTOMessageBuffer::FreeRTOMessageBuffer() {
    handle = xMessageBufferCreateStatic(backing.size(), backing.data(),
                                        &message_buffer_struct);
}

FreeRTOMessageBuffer::~FreeRTOMessageBuffer() { vMessageBufferDelete(handle); }

auto FreeRTOMessageBuffer::send(const uint8_t* buffer,
                                std::size_t buffer_length, uint32_t timeout)
    -> std::size_t {
    return xMessageBufferSendFromISR(handle, buffer, buffer_length, timeout);
}

auto FreeRTOMessageBuffer::send_from_isr(const uint8_t* buffer,
                                         std::size_t buffer_length)
    -> std::size_t {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    auto written_bytes = xMessageBufferSendFromISR(
        handle, buffer, buffer_length, &xHigherPriorityTaskWoken);

    if (written_bytes == message_length) {
        // see https://www.freertos.org/xMessageBufferSendFromISR.html for
        // explanation
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

auto FreeRTOMessageBuffer::receive(uint8_t* buffer, std::size_t buffer_length,
                                   uint32_t timeout) -> std::size_t {
    return xMessageBufferReceive(handle, buffer, buffer_length, timeout);
}

}  // namespace freertos_message_buffer