#pragma once

#include <concepts>

#include "common/core/message_queue.hpp"
#include "hepa-uv/core/constants.h"
#include "hepa-uv/core/messages.hpp"

namespace light_control_task {

using TaskMessage = led_control_task_messages::TaskMessage;

class LightControlInterface {
  public:
    LightControlInterface() = default;
    LightControlInterface(const LightControlInterface&) = delete;
    LightControlInterface(LightControlInterface&&) = delete;
    auto operator=(LightControlInterface&&) -> LightControlInterface& = delete;
    auto operator=(const LightControlInterface&)
        -> LightControlInterface& = delete;
    virtual ~LightControlInterface() = default;

    virtual auto set_led_power(uint8_t button, uint8_t led, uint32_t duty_cycle) -> void = 0;
};

class LightControlMessageHandler {
  public:
    LightControlMessageHandler(LightControlInterface& hardware)
        : _hardware(hardware) {}

    auto handle_message(const TaskMessage& message) -> void {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    auto handle(std::monostate&) -> void {}

    auto handle(const led_control_task_messages::PushButtonLED& msg) -> void {
        _hardware.set_led_power(msg.button, RED_LED, static_cast<uint32_t>(msg.r));
        _hardware.set_led_power(msg.button, GREEN_LED, static_cast<uint32_t>(msg.g));
        _hardware.set_led_power(msg.button, BLUE_LED, static_cast<uint32_t>(msg.b));
        _hardware.set_led_power(msg.button, WHITE_LED, static_cast<uint32_t>(msg.w));
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
        auto handler =
            LightControlMessageHandler(*hardware_handle);
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