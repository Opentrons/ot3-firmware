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

    virtual auto set_led_power(uint8_t id, uint32_t duty_cycle) -> void = 0;
};

/** Delay between each time update.*/
static constexpr uint32_t DELAY_MS = 5;

class LightControlMessageHandler {
  private:
    /** Integer for scaling light power.*/
    static constexpr uint32_t MAX_POWER = LED_PWM_WIDTH;
    /** Power level to set the deck LED to "on"*/
    static constexpr uint32_t DECK_LED_ON_POWER = 50;

  public:
    LightControlMessageHandler(LightControlInterface& hardware)
        : _hardware(hardware) {}

    auto handle_message(const TaskMessage& message) -> void {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    auto handle(std::monostate&) -> void {}

    auto handle(rearpanel::messages::UpdateLightControlMessage&) -> void {
        _hardware.set_led_power(DECK_LED, DECK_LED_ON_POWER);
        _hardware.set_led_power(RED_UI_LED, 0);
        _hardware.set_led_power(GREEN_UI_LED, 0);
        _hardware.set_led_power(BLUE_UI_LED, 0);
        _hardware.set_led_power(WHITE_UI_LED, 100);
    }

    LightControlInterface& _hardware;
};

/**
 * The task entry point.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class LightControlTask {
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
        auto handler = LightControlMessageHandler(*hardware_handle);
        TaskMessage message{};

        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
};

}  // namespace light_control_task