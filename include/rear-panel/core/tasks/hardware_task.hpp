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

    void check_estop_state(gpio_drive_hardware::GpioDrivePins* drive_pins) {
        // Monitor estop forced the actual pin toggle is handled
        // by the system task
        if (drive_pins->estop_forced && !estop_engaged) {
            estop_engaged = true;
        }
        // montitor the estop button(s)
        if ((gpio::is_set(drive_pins->estop_aux1_det) ||
             gpio::is_set(drive_pins->estop_aux2_det)) &&
            gpio::is_set(drive_pins->estop_in)) {
            if (!estop_engaged) {
                gpio::set(drive_pins->estop_out);
                estop_engaged = true;
            }
        } else {
            // release the estop line if the estop button is released
            // and we're not being forced
            if (estop_engaged && !drive_pins->estop_forced) {
                gpio::reset(drive_pins->estop_out);
                estop_engaged = false;
            }
        }
    }

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(
        gpio_drive_hardware::GpioDrivePins* drive_pins) {
        estop_engaged  = gpio::is_set(drive_pins->estop_out);
        for (;;) {
            check_estop_state(drive_pins);
            vTaskDelay(100);
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
    bool estop_engaged = false;
};

}  // namespace hardware_task
