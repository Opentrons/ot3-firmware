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
    virtual auto set_led_power(uint8_t id, uint32_t duty_cycle) -> void;
};

/**
 * The task entry point.
 */
template <template <class> class QueueImpl, typename LightControlHardware>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage> &&
    std::is_base_of_v<LightControlInterface, LightControlHardware>
class LightControlTask {
  private:
    static constexpr uint32_t MAX_POWER = LED_PWM_WIDTH;
    static constexpr uint32_t DELAY_MS = 5;

  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    LightControlTask(QueueType& queue, LightControlHardware& hardware)
        : queue{queue}, hardware{hardware} {}
    LightControlTask(const LightControlTask& c) = delete;
    LightControlTask(const LightControlTask&& c) = delete;
    auto operator=(const LightControlTask& c) = delete;
    auto operator=(const LightControlTask&& c) = delete;
    ~LightControlTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()() {
        auto handler = LightControlMessageHandler{*drive_pins};
        TaskMessage message{};
        TickType_t last_wake = xTaskGetTickCount();
        const TickType_t delay_ticks = pdMS_TO_TICKS(DELAY_MS);
        auto& queue_client = queue_client::get_main_queues();

        for (;;) {
            // With a timeout of 0, this will immediately return
            // if there isn't a message.
            uint8_t power = 0;
            if (queue.try_read(&message, 0)) {
                handler.handle_message(message);
            }

            policy->set_led_power(DECK_LED, MAX_POWER);
            policy->set_led_power(BLUE_UI_LED, power++);
            policy->set_led_power(WHITE_UI_LED, 0);
            policy->set_led_power(RED_UI_LED, 0);
            policy->set_led_power(GREEN_UI_LED, 0);

            // Sleep to drive LED update frequency
            vTaskDelayUntil(&last_wake, delay_ticks);
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    auto handle_message(const TaskMessage& message) -> void {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

    static auto handle(std::monostate&) -> void{};

    QueueType& queue;
    LightControlHardware& hardware;
};

}  // namespace light_control_task