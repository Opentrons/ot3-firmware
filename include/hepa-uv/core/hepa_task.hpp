#pragma once

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "common/firmware/gpio.hpp"
#include "hepa-uv/core/constants.h"
#include "hepa-uv/core/messages.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "hepa-uv/firmware/led_control_hardware.hpp"

namespace hepa_task {

using TaskMessage = interrupt_task_messages::TaskMessage;

class HepaMessageHandler {
  public:
    explicit HepaMessageHandler(
        gpio_drive_hardware::GpioDrivePins &drive_pins,
        led_control_hardware::LEDControlHardware &led_hardware)
        : drive_pins{drive_pins}, led_hardware{led_hardware} {
        // get current state
        hepa_push_button = gpio::is_set(drive_pins.hepa_push_button);
        // turn off the HEPA fan
        gpio::reset(drive_pins.hepa_on_off);
    }
    HepaMessageHandler(const HepaMessageHandler &) = delete;
    HepaMessageHandler(const HepaMessageHandler &&) = delete;
    auto operator=(const HepaMessageHandler &) -> HepaMessageHandler & = delete;
    auto operator=(const HepaMessageHandler &&)
        -> HepaMessageHandler && = delete;
    ~HepaMessageHandler() {}

    void handle_message(const TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(const std::monostate &) {}

    // Handle GPIO EXTI Interrupts here
    void visit(const interrupt_task_messages::GPIOInterruptChanged &m) {
        if (m.pin == drive_pins.hepa_push_button.pin) {
            hepa_push_button = !hepa_push_button;
            // handle state changes here
            if (hepa_push_button) {
                gpio::set(drive_pins.hepa_on_off);
                led_hardware.set_button_led_power(HEPA_BUTTON, 0, 50, 0, 0);
            } else {
                gpio::reset(drive_pins.hepa_on_off);
                led_hardware.set_button_led_power(HEPA_BUTTON, 0, 0, 0, 50);
            }
        }

        // TODO: send CAN message to host
    }

    // state tracking variables
    bool hepa_push_button = false;
    bool hepa_fan_on = false;

    gpio_drive_hardware::GpioDrivePins &drive_pins;
    led_control_hardware::LEDControlHardware &led_hardware;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class HepaTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    HepaTask(QueueType &queue) : queue{queue} {}
    HepaTask(const HepaTask &c) = delete;
    HepaTask(const HepaTask &&c) = delete;
    auto operator=(const HepaTask &c) = delete;
    auto operator=(const HepaTask &&c) = delete;
    ~HepaTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(
        gpio_drive_hardware::GpioDrivePins *drive_pins,
        led_control_hardware::LEDControlHardware *led_hardware) {
        auto handler = HepaMessageHandler{*drive_pins, *led_hardware};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
};
};  // namespace hepa_task
