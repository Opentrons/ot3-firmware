#pragma once

#include <concepts>

#include "common/core/message_queue.hpp"
#include "rear-panel/core/binary_parse.hpp"
#include "rear-panel/core/constants.h"
#include "rear-panel/core/lights/animation_handler.hpp"
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
/** Number of light actions allowed in an animation.*/
static constexpr size_t ANIMATION_BUFFER_SIZE = 64;

using Animation = lights::AnimationHandler<ANIMATION_BUFFER_SIZE>;

class LightControlMessageHandler {
  private:
    /** Integer for scaling light power.*/
    static constexpr uint32_t MAX_POWER = LED_PWM_WIDTH;
    /** Power level to set the deck LED to "on"*/
    static constexpr uint32_t DECK_LED_ON_POWER = 50;

  public:
    LightControlMessageHandler(LightControlInterface& hardware,
                               Animation& animation)
        : _hardware(hardware), _animation(animation) {}

    auto handle_message(const TaskMessage& message) -> void {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    auto handle(std::monostate&) -> void {}

    auto handle(rearpanel::messages::UpdateLightControlMessage&) -> void {
        auto color = _animation.animate(DELAY_MS);
        _hardware.set_led_power(DECK_LED, DECK_LED_ON_POWER);
        _hardware.set_led_power(RED_UI_LED, static_cast<uint32_t>(color.r));
        _hardware.set_led_power(GREEN_UI_LED, static_cast<uint32_t>(color.g));
        _hardware.set_led_power(BLUE_UI_LED, static_cast<uint32_t>(color.b));
        _hardware.set_led_power(WHITE_UI_LED, static_cast<uint32_t>(color.w));
    }

    auto handle(rearpanel::messages::AddLightActionRequest& msg) -> void {
        auto action = lights::Action{
            // The colors in the message are scaled [0,255], so we can just
            // keep that scale since it matches the max power setting
            .color{
                .r = static_cast<double>(msg.red),
                .g = static_cast<double>(msg.green),
                .b = static_cast<double>(msg.blue),
                .w = static_cast<double>(msg.white),
            },
            .transition = msg.transition,
            .transition_time_ms = msg.transition_time_ms};
        if (_animation.add_to_staging(action)) {
            ack();
        } else {
            nack();
        }
    }

    auto handle(rearpanel::messages::ClearLightActionStagingQueueRequest&)
        -> void {
        _animation.clear_staging();
        ack();
    }

    auto handle(rearpanel::messages::StartLightActionRequest& msg) -> void {
        _animation.start_staged_animation(msg.animation);
        ack();
    }

    static auto ack() -> void {
        queue_client::get_main_queues().send_host_comms_queue(
            rearpanel::messages::Ack{});
    }

    static auto nack() -> void {
        queue_client::get_main_queues().send_host_comms_queue(
            rearpanel::messages::AckFailed{});
    }

    LightControlInterface& _hardware;
    Animation& _animation;
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
    [[noreturn]] void operator()(LightControlInterface* hardware_handle,
                                 Animation* animation_handle) {
        auto handler =
            LightControlMessageHandler(*hardware_handle, *animation_handle);
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