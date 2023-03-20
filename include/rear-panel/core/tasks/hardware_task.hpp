/*
 * Interface for the firmware-specific hardware tasks
 */
#pragma once

#include "FreeRTOS.h"
#include "common/firmware/gpio.hpp"
#include "rear-panel/core/messages.hpp"
#include "common/core/debounce.hpp"
#include "rear-panel/core/queues.hpp"
#include "rear-panel/core/tasks.hpp"
#include "rear-panel/firmware/gpio_drive_hardware.hpp"

namespace hardware_task {

using TaskMessage = rearpanel::messages::HardwareTaskMessage;
/*
 * The task entry point.
 */

struct GpioInputState {
    bool estop_in = false;
    bool estop_aux1_det = false;
    bool estop_aux2_det = false;
    // TODO:
    // door switch
    // Aux device present
    auto operator==(const GpioInputState& other) const -> bool = default;
};

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

    auto send_state_change_messages(GpioInputState& current,
                                    GpioInputState& previous) -> void {
        auto queue_client = queue_client::get_main_queues();
        if (current.estop_in != previous.estop_in) {
            queue_client.send_host_comms_queue(
                rearpanel::messages::EstopStateChange{.engaged =
                                                          current.estop_in});
        }
        if (current.estop_aux1_det != previous.estop_aux1_det ||
            current.estop_aux2_det != previous.estop_aux2_det) {
            queue_client.send_host_comms_queue(
                rearpanel::messages::EstopButtonDetectionChange{
                    .aux1_present = current.estop_aux1_det,
                    .aux2_present = current.estop_aux2_det});
        }
    }

    auto get_new_state(gpio_drive_hardware::GpioDrivePins* drive_pins)
        -> GpioInputState {
        estop_in_bouncer.debounce_update(gpio::is_set(drive_pins->estop_in));
        estop_aux1_bouncer.debounce_update(
            gpio::is_set(drive_pins->estop_aux1_det));
        estop_aux2_bouncer.debounce_update(
            gpio::is_set(drive_pins->estop_aux2_det));
        // TODO:
        // door switch
        // Aux device present

        return GpioInputState{
            .estop_in = estop_in_bouncer.debounce_state(),
            .estop_aux1_det = estop_aux1_bouncer.debounce_state(),
            .estop_aux2_det = estop_aux2_bouncer.debounce_state()
            // TODO:
            // door switch
            // Aux device present
        };
    }
    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(
        gpio_drive_hardware::GpioDrivePins* drive_pins) {
        /// This task monitors all of the various gpio inputs
        // initialize the starting state of the board
        auto current_state = GpioInputState{
            .estop_in = gpio::is_set(drive_pins->estop_in),
            .estop_aux1_det = gpio::is_set(drive_pins->estop_aux1_det),
            .estop_aux2_det = gpio::is_set(drive_pins->estop_aux2_det)
            // TODO:
            // door switch
            // Aux device present
        };
        for (;;) {
            auto new_state = get_new_state(drive_pins);
            if (current_state != new_state) {
                // if the state has changed send a notification
                send_state_change_messages(new_state, current_state);
                current_state = new_state;
            }
            vTaskDelay(100);
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
    debouncer::Debouncer estop_in_bouncer = debouncer::Debouncer{};
    debouncer::Debouncer estop_aux1_bouncer = debouncer::Debouncer{};
    debouncer::Debouncer estop_aux2_bouncer = debouncer::Debouncer{};
    bool estop_engaged = false;
};

}  // namespace hardware_task
