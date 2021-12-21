#pragma once

#include <array>

#include "FreeRTOS.h"
#include "queue.h"

namespace freertos_message_queue {

template <typename Message, size_t queue_size = 10>
class FreeRTOSMessageQueue {
  public:
    static auto constexpr max_delay = portMAX_DELAY;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    explicit FreeRTOSMessageQueue(const char* name) : FreeRTOSMessageQueue() {
        vQueueAddToRegistry(queue, name);
    }
    explicit FreeRTOSMessageQueue()
        : queue_control_structure(),
          queue_data_structure(),
          queue(xQueueCreateStatic(queue_size, sizeof(Message),
                                   queue_data_structure.data(),
                                   &queue_control_structure)) {}
    auto operator=(FreeRTOSMessageQueue&) -> FreeRTOSMessageQueue& = delete;
    auto operator=(FreeRTOSMessageQueue&&) -> FreeRTOSMessageQueue&& = delete;
    FreeRTOSMessageQueue(FreeRTOSMessageQueue&) = delete;
    FreeRTOSMessageQueue(FreeRTOSMessageQueue&&) = delete;

    ~FreeRTOSMessageQueue() {
        vQueueUnregisterQueue(queue);
        vQueueDelete(queue);
    }

    template <typename TimeoutType>
    requires std::is_integral_v<TimeoutType>
    auto try_write(const Message& message, TimeoutType timeout_ticks) -> bool {
        return xQueueSendToBack(queue, &message, timeout_ticks) == pdTRUE;
    }

    auto try_write(const Message& message) -> bool {
        return try_write(message, 0);
    }

    template <typename TimeoutType>
    requires std::is_integral_v<TimeoutType>
    auto try_read(Message* message, TimeoutType timeout_ticks) -> bool {
        return xQueueReceive(queue, message, timeout_ticks);
    }

    auto try_read(Message* message) -> bool { return try_read(message, 0); }

    [[nodiscard]] auto try_write_isr(const Message& message) -> bool {
        BaseType_t higher_woken = pdFALSE;
        auto sent = xQueueSendFromISR(queue, &message, &higher_woken);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        portYIELD_FROM_ISR(higher_woken);
        return sent;
    }

    auto try_read_isr(Message* message) const -> bool {
        BaseType_t higher_woken = pdFALSE;
        auto recv = xQueueReceiveFromISR(queue, message, &higher_woken);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        portYIELD_FROM_ISR(higher_woken);
        return recv;
    }

    [[nodiscard]] auto has_message() const -> bool {
        return uxQueueMessagesWaiting(queue) != 0;
    }

    [[nodiscard]] auto has_message_isr() const -> bool {
        return uxQueueMessagesWaitingFromISR(queue) != 0;
    }

    [[nodiscard]] auto peek_isr(Message* message) const -> bool {
        return xQueuePeekFromISR(queue, message) == pdTRUE;
    }

    template <typename TimeoutType>
    requires std::is_integral_v<TimeoutType>
    [[nodiscard]] auto peek(Message* message, TimeoutType timeout_ticks) const
        -> bool {
        return xQueuePeek(queue, message, timeout_ticks) == pdTRUE;
    }

    [[nodiscard]] auto peek(Message* message) const -> bool {
        return peek(message, 0);
    }

    void reset() { xQueueReset(queue); }

  private:
    StaticQueue_t queue_control_structure;
    std::array<uint8_t, queue_size * sizeof(Message)> queue_data_structure;
    QueueHandle_t queue;
};

}  // namespace freertos_message_queue
