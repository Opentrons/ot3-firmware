#pragma once

#include <array>
#include <cstdint>

#include "FreeRTOS.h"
#include "common/core/bit_utils.hpp"
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
    FreeRTOMessageBuffer& operator=(FreeRTOMessageBuffer&) = delete;
    FreeRTOMessageBuffer&& operator=(FreeRTOMessageBuffer&&) = delete;
    FreeRTOMessageBuffer(FreeRTOMessageBuffer&) = delete;
    FreeRTOMessageBuffer(FreeRTOMessageBuffer&&) = delete;

    ~FreeRTOMessageBuffer();

    template <typename Iterator, typename Limit>
    requires bit_utils::ByteIterator<Iterator> &&
        std::sentinel_for<Limit, Iterator>
    auto send(const Iterator iter, const Limit limit, uint32_t timeout)
        -> std::size_t {
        return xMessageBufferSend(handle, iter, limit - iter, timeout);
    }

    template <typename Iterator, typename Limit>
    requires bit_utils::ByteIterator<Iterator> &&
        std::sentinel_for<Limit, Iterator>
    auto send_from_isr(const Iterator iter, const Limit limit) -> std::size_t {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        auto buffer_length = limit - iter;

        auto written_bytes = xMessageBufferSendFromISR(
            handle, iter, buffer_length, &xHigherPriorityTaskWoken);

        if (written_bytes == buffer_length) {
            // see https://www.freertos.org/xMessageBufferSendFromISR.html for
            // explanation
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        return written_bytes;
    }

    template <typename Iterator, typename Limit>
    requires bit_utils::ByteIterator<Iterator> &&
        std::sentinel_for<Limit, Iterator>
    auto receive(Iterator iter, Limit limit, uint32_t timeout) -> std::size_t {
        return xMessageBufferReceive(handle, iter, limit - iter, timeout);
    }

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
FreeRTOMessageBuffer<BufferSize>::~FreeRTOMessageBuffer() {
    vMessageBufferDelete(handle);
}

}  // namespace freertos_message_buffer