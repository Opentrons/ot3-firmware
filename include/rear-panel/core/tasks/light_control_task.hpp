#pragma once

#include <concepts>

#include "common/core/message_queue.hpp"
#include "rear-panel/core/binary_parse.hpp"
#include "rear-panel/core/constants.h"
#include "rear-panel/core/messages.hpp"
#include "rear-panel/core/queues.hpp"

namespace light_control_task {

using TaskMessage = rearpanel::messages::LightControlTaskMessage;

class LightControlInterface {
  public:
    LightControlInterface() = default;
    LightControlInterface(const LightControlInterface&) = delete;
    LightControlInterface(LightControlInterface&&) = delete;
    auto operator=(LightControlInterface&&) -> LightControlInterface& = delete;
    auto operator=(const LightControlInterface&)
        -> LightControlInterface& = delete;
    virtual ~LightControlInterface() = default;

    virtual auto set_led_power(uint8_t id, uint32_t duty_cycle) -> void;
};

/**
 * The task entry point.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class LightControlTask {
  private:
    static constexpr uint32_t MAX_POWER = LED_PWM_WIDTH;
    static constexpr uint32_t DELAY_MS = 5;

  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    LightControlTask(QueueType& queue) : queue{queue} {}
    LightControlTask(const LightControlTask& c) = delete;
    LightControlTask(const LightControlTask&& c) = delete;
    auto operator=(const LightControlTask& c) = delete;
    auto operator=(const LightControlTask&& c) = delete;
    ~LightControlTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(LightControlInterface* hardware_handle) {
        auto& hardware = *hardware_handle;
        TaskMessage message{};
        TickType_t last_wake = xTaskGetTickCount();
        const TickType_t delay_ticks = pdMS_TO_TICKS(DELAY_MS);
        uint8_t power = 0;

        for (;;) {
            // With a timeout of 0, this will immediately return
            // if there isn't a message.
            if (queue.try_read(&message, 0)) {
                handle_message(message);
            }

            hardware.set_led_power(DECK_LED, 50);
            hardware.set_led_power(BLUE_UI_LED, power++);
            hardware.set_led_power(WHITE_UI_LED, 0);
            hardware.set_led_power(RED_UI_LED, 0);
            hardware.set_led_power(GREEN_UI_LED, 0);

            // Sleep to drive LED update frequency
            vTaskDelayUntil(&last_wake, delay_ticks);
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    auto handle_message(const TaskMessage& message) -> void {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

    static auto handle(std::monostate&) -> void {}

    QueueType& queue;
};

}  // namespace light_control_task