#pragma once

#include <array>

#include "FreeRTOS.h"
#include "queue.h"

namespace freertos_message_queue {

template <typename Message, size_t queue_size = 10>
class FreeRTOSMessageQueue {
  public:
    explicit FreeRTOSMessageQueue(uint8_t notification_bit, const char* name)
        : FreeRTOSMessageQueue(notification_bit) {
        vQueueAddToRegistry(queue, name);
    }
    explicit FreeRTOSMessageQueue(uint8_t notification_bit)
        : queue_control_structure(),
          queue_data_structure(),
          queue(xQueueCreateStatic(queue_size, sizeof(Message),
                                   queue_data_structure.data(),
                                   &queue_control_structure)),
          sent_bit(notification_bit) {}
    FreeRTOSMessageQueue& operator=(FreeRTOSMessageQueue&) = delete;
    FreeRTOSMessageQueue&& operator=(FreeRTOSMessageQueue&&) = delete;
    FreeRTOSMessageQueue(FreeRTOSMessageQueue&) = delete;
    FreeRTOSMessageQueue(FreeRTOSMessageQueue&&) = delete;

    ~FreeRTOSMessageQueue() {
        vQueueUnregisterQueue(queue);
        vQueueDelete(queue);
    }

    [[nodiscard]] auto try_write(const Message& message,
                                 const uint32_t timeout_ticks = 0) -> bool {
        auto sent = xQueueSendToBack(queue, &message, timeout_ticks) == pdTRUE;
        //            if (sent) {
        //                xTaskNotify(receiver_handle, 1 << sent_bit, eSetBits);
        //            }
        return sent;
    }

    auto try_read(Message* message, uint32_t timeout_ticks = 0) -> bool {
        return xQueueReceive(queue, message, timeout_ticks);
    }

    [[nodiscard]] auto try_write_isr(const Message& message) -> bool {
        BaseType_t higher_woken = pdFALSE;
        auto sent = xQueueSendFromISR(queue, &message, &higher_woken);
        portYIELD_FROM_ISR(  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
            higher_woken);
        return sent;
    }

    auto try_read_isr(Message* message) const -> bool {
        BaseType_t higher_woken = pdFALSE;
        return xQueueReceiveFromISR(queue, message, &higher_woken) == pdTRUE;
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

    void reset() { xQueueReset(queue); }

  private:
    StaticQueue_t queue_control_structure;
    std::array<uint8_t, queue_size * sizeof(Message)> queue_data_structure;
    QueueHandle_t queue;
    uint8_t sent_bit;
};

}  // namespace freertos_message_queue
