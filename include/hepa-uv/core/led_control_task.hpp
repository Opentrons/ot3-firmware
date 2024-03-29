#pragma once

#include <concepts>

#include "common/core/message_queue.hpp"
#include "hepa-uv/core/constants.h"
#include "hepa-uv/core/interfaces.hpp"
#include "hepa-uv/core/messages.hpp"

namespace led_control_task {

using namespace led_control;

using TaskMessage = led_control_task_messages::TaskMessage;

class LEDControlMessageHandler {
  public:
    LEDControlMessageHandler(LEDControlInterface& hardware)
        : _hardware(hardware) {}

    auto handle_message(const TaskMessage& message) -> void {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    auto handle(std::monostate&) -> void {}

    auto handle(const led_control_task_messages::PushButtonLED& msg) -> void {
        // Sets the Push button LED colors
        _hardware.set_button_led_power(msg.button, static_cast<uint32_t>(msg.r),
                                       static_cast<uint32_t>(msg.g),
                                       static_cast<uint32_t>(msg.b),
                                       static_cast<uint32_t>(msg.w));
    }

    LEDControlInterface& _hardware;
};

/**
 * The task entry point.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class LEDControlTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    LEDControlTask(QueueType& queue) : queue{queue} {}
    LEDControlTask(const LEDControlTask& c) = delete;
    LEDControlTask(const LEDControlTask&& c) = delete;
    auto operator=(const LEDControlTask& c) = delete;
    auto operator=(const LEDControlTask&& c) = delete;
    ~LEDControlTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(LEDControlInterface* hardware_handle) {
        auto handler = LEDControlMessageHandler(*hardware_handle);
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

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_led_control_message(m)};
};

}  // namespace led_control_task