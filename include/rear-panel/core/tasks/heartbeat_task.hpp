/*
 * Heartbeat blink task.
 */
#pragma once

#include "FreeRTOS.h"
#include "common/firmware/gpio.hpp"
#include "rear-panel/core/queues.hpp"
#include "rear-panel/core/tasks.hpp"
#include "rear-panel/firmware/gpio_drive_hardware.hpp"

namespace heartbeat_task {

using TaskMessage = std::variant<std::monostate>;
/*
 * The task entry point.
 */

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class HeartbeatTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    HeartbeatTask(QueueType& queue) : queue{queue} {}
    HeartbeatTask(const HeartbeatTask& c) = delete;
    HeartbeatTask(const HeartbeatTask&& c) = delete;
    auto operator=(const HeartbeatTask& c) = delete;
    auto operator=(const HeartbeatTask&& c) = delete;
    ~HeartbeatTask() = default;

    void heartbeat(gpio::PinConfig heartbeat_led) {
#if !(PCBA_PRIMARY_REVISION == 'b' && PCBA_SECONDARY_REVISION == '1')
        if (gpio::is_set(heartbeat_led)) {
            gpio::reset(heartbeat_led);
        } else {
            gpio::set(heartbeat_led);
        }
#endif
    }
    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(
        gpio_drive_hardware::GpioDrivePins* drive_pins) {
        for (;;) {
            vTaskDelay(200 * portTICK_PERIOD_MS);
            heartbeat(drive_pins->heartbeat_led);
            vTaskDelay(350 * portTICK_PERIOD_MS);
            heartbeat(drive_pins->heartbeat_led);
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
};

}  // namespace heartbeat_task
