#pragma once

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "hepa-uv/core/constants.h"
#include "hepa-uv/core/led_control_task.hpp"
#include "hepa-uv/core/messages.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "ot_utils/freertos/freertos_timer.hpp"

namespace uv_task {

// How long to keep the UV light on in ms.
// static constexpr uint32_t DELAY_MS = 1000 * 60 * 15;  // 15 minutes
static constexpr uint32_t DELAY_MS = 1000 * 30;  // 30s

using TaskMessage = interrupt_task_messages::TaskMessage;

template <led_control_task::TaskClient LEDControlClient>
class UVMessageHandler {
  public:
    explicit UVMessageHandler(gpio_drive_hardware::GpioDrivePins &drive_pins,
                              LEDControlClient &led_control_client)
        : drive_pins{drive_pins},
          led_control_client{led_control_client},
          _timer(
              "UVTask", [ThisPtr = this] { ThisPtr->timer_callback(); },
              DELAY_MS) {
        // get current state
        uv_push_button = gpio::is_set(drive_pins.uv_push_button);
        door_closed = gpio::is_set(drive_pins.door_open);
        reed_switch_set = gpio::is_set(drive_pins.reed_switch);
        // turn off UV Ballast
        gpio::reset(drive_pins.uv_on_off);
    }
    UVMessageHandler(const UVMessageHandler &) = delete;
    UVMessageHandler(const UVMessageHandler &&) = delete;
    auto operator=(const UVMessageHandler &) -> UVMessageHandler & = delete;
    auto operator=(const UVMessageHandler &&) -> UVMessageHandler && = delete;
    ~UVMessageHandler() {}

    void handle_message(const TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    // call back to turn off the UV ballast
    auto timer_callback() -> void {
        gpio::reset(drive_pins.uv_on_off);
        // Set the push button LED's to idle (white)
        led_control_client.send_led_control_message(
            led_control_task_messages::PushButtonLED(UV_BUTTON, 0, 0, 0, 50));
        uv_push_button = false;
        uv_fan_on = false;
    }

    void visit(const std::monostate &) {}

    // Handle GPIO EXTI Interrupts here
    void visit(const interrupt_task_messages::GPIOInterruptChanged &m) {
        if (m.pin == drive_pins.hepa_push_button.pin) {
            // ignore hepa push button presses
            return;
        }

        // update states
        if (m.pin == drive_pins.uv_push_button.pin) {
            uv_push_button = !uv_push_button;
        } else if (m.pin == drive_pins.door_open.pin) {
            door_closed = gpio::is_set(drive_pins.door_open);
        } else if (m.pin == drive_pins.reed_switch.pin) {
            reed_switch_set = gpio::is_set(drive_pins.reed_switch);
        }

        // reset push button state if the door is opened or the reed switch is
        // not set
        if (!door_closed || !reed_switch_set) {
            uv_push_button = false;
            gpio::set(drive_pins.uv_on_off);
            // TODO: Pulse this instead of solid blue
            // Set the push button LED's to user intervention (blue)
            led_control_client.send_led_control_message(
                led_control_task_messages::PushButtonLED(UV_BUTTON, 0, 0, 50,
                                                         0));
            if (_timer.is_running()) _timer.stop();
            return;
        }

        // set the UV Ballast
        if (uv_push_button) {
            gpio::set(drive_pins.uv_on_off);
            // Set the push button LED's to running (green)
            led_control_client.send_led_control_message(
                led_control_task_messages::PushButtonLED(UV_BUTTON, 0, 50, 0,
                                                         0));
            if (_timer.is_running()) _timer.stop();
            _timer.start();
            uv_fan_on = true;
        } else {
            gpio::reset(drive_pins.uv_on_off);
            // Set the push button LED's to idle (white)
            led_control_client.send_led_control_message(
                led_control_task_messages::PushButtonLED(UV_BUTTON, 0, 0, 0,
                                                         50));
            if (_timer.is_running()) _timer.stop();
            uv_fan_on = false;
        }

        // TODO: send CAN message to host
    }

    // state tracking variables
    bool door_closed = false;
    bool reed_switch_set = false;
    bool uv_push_button = false;
    bool uv_fan_on = false;

    gpio_drive_hardware::GpioDrivePins &drive_pins;
    LEDControlClient &led_control_client;
    ot_utils::freertos_timer::FreeRTOSTimer _timer;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class UVTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    UVTask(QueueType &queue) : queue{queue} {}
    UVTask(const UVTask &c) = delete;
    UVTask(const UVTask &&c) = delete;
    auto operator=(const UVTask &c) = delete;
    auto operator=(const UVTask &&c) = delete;
    ~UVTask() = default;

    /**
     * Task entry point.
     */
    template <led_control_task::TaskClient LEDControlClient>
    [[noreturn]] void operator()(gpio_drive_hardware::GpioDrivePins *drive_pins,
                                 LEDControlClient *led_control_client) {
        auto handler = UVMessageHandler{*drive_pins, *led_control_client};
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
};  // namespace uv_task
