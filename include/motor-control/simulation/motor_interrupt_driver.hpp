#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.h"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"

namespace motor_interrupt_driver {

template <move_status_reporter_task::TaskClient StatusClient>
class MotorInterruptDriver {
  public:
    MotorInterruptDriver(
        freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>& q,
        motor_handler::MotorInterruptHandler<
            freertos_message_queue::FreeRTOSMessageQueue, StatusClient>& h,
        sim_motor_hardware_iface::SimMotorHardwareIface& iface)
        : task_entry{q, h, iface}, task(task_entry) {
        task.start(5, "sim_motor_isr");
    }

  private:
    struct TaskEntry {
        TaskEntry(
            freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>&
                q,
            motor_handler::MotorInterruptHandler<
                freertos_message_queue::FreeRTOSMessageQueue, StatusClient>& h,
            sim_motor_hardware_iface::SimMotorHardwareIface& motor_iface)
            : queue{q}, handler{h}, iface{motor_iface} {}

        void operator()() {
            while (true) {
                auto move = motor_messages::Move{};
                if (queue.peek(&move, queue.max_delay)) {
                    LOG("Enabling motor interrupt handler for group %d, seq "
                        "%d, duration %ld",
                        move.group_id, move.seq_id, move.duration);
                    do {
                        if (queue.peek(&move, 0)) {
                            if (move.stop_condition ==
                                motor_messages::MoveStopCondition::
                                    limit_switch) {
                                iface.trigger_limit_switch();
                                LOG("Received Home Request, triggering limit "
                                    "switch\n");
                            }
                        }
                        handler.run_interrupt();

                    } while (handler.has_active_move);
                    LOG("Move completed. Stopping interrupt simulation..");
                }
            }
        }
        freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>&
            queue;
        motor_handler::MotorInterruptHandler<
            freertos_message_queue::FreeRTOSMessageQueue, StatusClient>&
            handler;
        sim_motor_hardware_iface::SimMotorHardwareIface& iface;
    };

    TaskEntry task_entry;
    freertos_task::FreeRTOSTask<128, TaskEntry> task;
};

}  // namespace motor_interrupt_driver
