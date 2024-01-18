#pragma once

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "messages.hpp"

namespace uv_task {

using TaskMessage = interrupt_task_messages::TaskMessage;

class UVMessageHandler {
  public:
    explicit UVMessageHandler(gpio_drive_hardware::GpioDrivePins &drive_pins)
        : drive_pins{drive_pins} {}
    UVMessageHandler(const UVMessageHandler &) = delete;
    UVMessageHandler(const UVMessageHandler &&) = delete;
    auto operator=(const UVMessageHandler &) -> UVMessageHandler & = delete;
    auto operator=(const UVMessageHandler &&)
        -> UVMessageHandler && = delete;
    ~UVMessageHandler() {}

    void handle_message(const TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(const std::monostate &) {}

    // Handle GPIO EXTI Interrupts here
    void visit(const interrupt_task_messages::GPIOInterruptChanged &m) {
        if (m.pin == drive_pins.uv_push_button.pin) {
            uv_push_button = !uv_push_button;
            // handle state changes here
            if (uv_push_button) {
                gpio::set(drive_pins.uv_on_off);
            } else {
                gpio::reset(drive_pins.uv_on_off);
            }
        }

        // TODO: send CAN message to host
    }

    // state tracking variables
    // TODO: add reed, door_open, etc
    bool uv_push_button = false;
    bool uv_fan_on = false;

    gpio_drive_hardware::GpioDrivePins &drive_pins;
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
