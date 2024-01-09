/*
 * Heartbeat blink task.
 */
#pragma once

#include "FreeRTOS.h"
#include "common/firmware/gpio.hpp"
#include "hepa-uv/core/queues.hpp"
#include "hepa-uv/core/tasks.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"

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

    void heartbeat(gpio::PinConfig push_button_led, uint8_t value) {
        if (value == 0) {
            gpio::set(push_button_led);
        } else {
            gpio::reset(push_button_led);
        }
    }
    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(
        gpio_drive_hardware::GpioDrivePins* drive_pins) {
        for (;;) {
            uint8_t door_open = gpio::read_pin(drive_pins->door_open);
            uint8_t reed_switch = gpio::read_pin(drive_pins->reed_switch);
            uint8_t push_button = gpio::read_pin(drive_pins->push_button);
            uint8_t led_value = (door_open == 0 && push_button == 1) ? 1 : 0;
            heartbeat(drive_pins->push_button_led, led_value);
            vTaskDelay(200 * portTICK_PERIOD_MS);
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
};

}  // namespace heartbeat_task
