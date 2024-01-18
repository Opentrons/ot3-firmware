#pragma once

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "messages.hpp"
#include "ot_utils/freertos/freertos_timer.hpp"

namespace uv_task {

// How long to keep the UV light on in ms.
static constexpr uint32_t DELAY_MS = 1000 * 60 * 15;  // 15 minutes

using TaskMessage = interrupt_task_messages::TaskMessage;

class UVMessageHandler {
  public:
    explicit UVMessageHandler(gpio_drive_hardware::GpioDrivePins &drive_pins)
        : drive_pins{drive_pins},
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
        if (!door_closed || !reed_switch_set) uv_push_button = false;

        // set the UV Ballast
        if (door_closed && reed_switch_set && uv_push_button) {
            gpio::set(drive_pins.uv_on_off);
            // start the turn off timer
            if (_timer.is_running()) _timer.stop();
            _timer.start();
            uv_fan_on = true;
        } else {
            gpio::reset(drive_pins.uv_on_off);
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
    [[noreturn]] void operator()(
        gpio_drive_hardware::GpioDrivePins *drive_pins) {
        auto handler = UVMessageHandler{*drive_pins};
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
