/*
 * Interface for the firmware-specific hardware tasks
 */
#pragma once

#include "FreeRTOS.h"
#include "common/firmware/gpio.hpp"
#include "rear-panel/core/messages.hpp"
#include "rear-panel/core/tasks.hpp"
#include "rear-panel/firmware/gpio_drive_hardware.hpp"

namespace hardware_task {

using TaskMessage = rearpanel::messages::HardwareTaskMessage;
/*
 * The task entry point.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class HardwareTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    HardwareTask(QueueType& queue) : queue{queue} {}
    HardwareTask(const HardwareTask& c) = delete;
    HardwareTask(const HardwareTask&& c) = delete;
    auto operator=(const HardwareTask& c) = delete;
    auto operator=(const HardwareTask&& c) = delete;
    ~HardwareTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(
        gpio_drive_hardware::GpioDrivePins* drive_pins) {
        for (;;) {
            if ((gpio::is_set(drive_pins->estop_aux1_det) ||
                 gpio::is_set(drive_pins->estop_aux2_det)) &&
                gpio::is_set(drive_pins->estop_in)) {
                gpio::set(drive_pins->estop_out);
            } else {
                gpio::reset(drive_pins->estop_out);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
};

}  // namespace hardware_task
