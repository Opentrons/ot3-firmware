#pragma once

#include <cstdint>
#include <array>
#include "message_buffer.h"

namespace freertos_message_buffer {

/**
 * A MessageBuffer backed by a FreeRTOS message buffer
 * @tparam BufferSize The buffer size in bytes.
 */
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
    StaticMessageBuffer_t message_buffer{};
    MessageBufferHandle_t handle{};
    std::array<uint8_t, BufferSize> backing{};
};

template <std::size_t BufferSize>
FreeRTOMessageBuffer<BufferSize>::FreeRTOMessageBuffer() {
    handle = xMessageBufferCreateStatic(backing.size(), backing.data(),
                                        &message_buffer);
}

template <std::size_t BufferSize>
FreeRTOMessageBuffer<BufferSize>::~FreeRTOMessageBuffer() { vMessageBufferDelete(handle); }

template <std::size_t BufferSize>
auto FreeRTOMessageBuffer<BufferSize>::send(const uint8_t* buffer,
                                std::size_t buffer_length, uint32_t timeout)
    -> std::size_t {
    return xMessageBufferSend(handle, buffer, buffer_length, timeout);
}

template <std::size_t BufferSize>
auto FreeRTOMessageBuffer<BufferSize>::send_from_isr(const uint8_t* buffer,
                                         std::size_t buffer_length)
    -> std::size_t {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    auto written_bytes = xMessageBufferSendFromISR(
        handle, buffer, buffer_length, &xHigherPriorityTaskWoken);

    if (written_bytes == buffer_length) {
        // see https://www.freertos.org/xMessageBufferSendFromISR.html for
        // explanation
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return written_bytes;
}

template <std::size_t BufferSize>
auto FreeRTOMessageBuffer<BufferSize>::receive(uint8_t* buffer, std::size_t buffer_length,
                                   uint32_t timeout) -> std::size_t {
    return xMessageBufferReceive(handle, buffer, buffer_length, timeout);
}

}  // namespace freertos_message_buffer