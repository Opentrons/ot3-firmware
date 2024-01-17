#pragma once

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "messages.hpp"

namespace hepa_task {

using TaskMessage = interrupt_task_messages::TaskMessage;

class HepaMessageHandler {
  public:
    explicit HepaMessageHandler(gpio_drive_hardware::GpioDrivePins &drive_pins)
        : drive_pins{drive_pins} {}
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
            } else {
                gpio::reset(drive_pins.hepa_on_off);
            }
        }

        // TODO: send CAN message to host
    }

    // state tracking variables
    bool hepa_push_button = false;
    bool hepa_fan_on = false;

    gpio_drive_hardware::GpioDrivePins &drive_pins;
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
        gpio_drive_hardware::GpioDrivePins *drive_pins) {
        auto handler = HepaMessageHandler{*drive_pins};
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
