#include <array>
#include <concepts>

#include "queue.h"

namespace freertos_message_queue {
    template<class T, typename Message, size_t queue_size = 10>
    concept FreeRTOSGenericQueue = requires(T t, Message* msg_write, Message& msg_receive, uint32_t timeout_ticks) {
        {t.try_write(msg_write, timeout_ticks)};
        {t.try_read(msg_receive, timeout_ticks)};
        {t.has_message()};
        {t.pop()};
        {t.peek()};
    };

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
              sent_bit(notification_bit){}
        FreeRTOSMessageQueue& operator=(FreeRTOSMessageQueue&) = delete;
        FreeRTOSMessageQueue&& operator=(FreeRTOSMessageQueue&&) = delete;
        FreeRTOSMessageQueue(FreeRTOSMessageQueue&) = delete;
        FreeRTOSMessageQueue(FreeRTOSMessageQueue&&) = delete;

        ~FreeRTOSMessageQueue() {
            vQueueUnregisterQueue(queue);
            vQueueDelete(queue);
        }

        void try_write(const Message* message, const uint32_t timeout_ticks = 0) {
            auto sent = xQueueSendToBack(queue, message, timeout_ticks) == pdTRUE;
            if (sent) {
                xTaskNotify(receiver_handle, 1 << sent_bit, eSetBits);
            }
            return sent;
        }

        auto try_read(Message& message, uint32_t timeout_ticks = 0) { return xQueueReceive(queue, message, timeout_ticks); }

        [[nodiscard]] auto try_send_from_isr(const Message& message) -> bool {
            BaseType_t higher_woken = pdFALSE;
            auto sent = xQueueSendFromISR(queue, &message, &higher_woken);
            portYIELD_FROM_ISR(  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                higher_woken);
            return sent;
        }

        [[nodiscard]] auto has_message() const -> bool {
            return uxQueueMessagesWaiting(queue) != 0;
        }

      private:
        StaticQueue_t queue_control_structure;
        std::array<uint8_t, queue_size * sizeof(Message)> queue_data_structure;
        QueueHandle_t queue;
        TaskHandle_t receiver_handle;
        uint8_t sent_bit;
    };

    template<typename Message, size_t queue_size = 10>
    class FreeRTOSMessageQueueISR {
      public:
        explicit FreeRTOSMessageQueueISR(uint8_t notification_bit, const char* name)
            : FreeRTOSMessageQueueISR(notification_bit) {
            vQueueAddToRegistry(queue, name);
        }
        explicit FreeRTOSMessageQueueISR(uint8_t notification_bit)
        : queue_control_structure(),
        queue_data_structure(),
        queue(xQueueCreateStatic(queue_size, sizeof(Message),
                                 queue_data_structure.data(),
                                 &queue_control_structure)),
        sent_bit(notification_bit){}
        FreeRTOSMessageQueueISR& operator=(FreeRTOSMessageQueueISR&) = delete;
        FreeRTOSMessageQueueISR&& operator=(FreeRTOSMessageQueueISR&&) = delete;
        FreeRTOSMessageQueueISR(FreeRTOSMessageQueueISR&) = delete;
        FreeRTOSMessageQueueISR(FreeRTOSMessageQueueISR&&) = delete;

        ~FreeRTOSMessageQueueISR() {
            vQueueUnregisterQueue(queue);
            vQueueDelete(queue);
        }

        [[nodiscard]] auto try_write(const Message& message) -> bool {
            BaseType_t higher_woken = pdFALSE;
            auto sent = xQueueSendFromISR(queue, &message, &higher_woken);
            portYIELD_FROM_ISR(  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                higher_woken);
            return sent;
        }

        [[nodiscard]] auto try_read(const Message& message) -> bool {
            BaseType_t higher_woken = pdFALSE;
            auto sent = xQueueSendFromISR(queue, &message, &higher_woken);
            portYIELD_FROM_ISR(  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                higher_woken);
            return sent;
        }

        [[nodiscard]] auto has_message() const -> bool {
            return uxQueueMessagesWaiting(queue) != 0;
        }

      private:
        StaticQueue_t queue_control_structure;
        std::array<uint8_t, queue_size * sizeof(Message)> queue_data_structure;
        QueueHandle_t queue;
        TaskHandle_t receiver_handle;
        uint8_t sent_bit;
    };
}
