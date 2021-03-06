#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.h"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"

namespace motor_interrupt_driver {

template <class StatusClient, typename MotorMoveMessage, class MotorHardware>
class MotorInterruptDriver {
    using InterruptQueue =
        freertos_message_queue::FreeRTOSMessageQueue<MotorMoveMessage>;
    using InterruptHandler = motor_handler::MotorInterruptHandler<
        freertos_message_queue::FreeRTOSMessageQueue, StatusClient,
        MotorMoveMessage>;

  public:
    MotorInterruptDriver(InterruptQueue& q, InterruptHandler& h,
                         MotorHardware& iface)
        : task_entry{q, h, iface}, task(task_entry) {
        task.start(5, "sim_motor_isr");
    }

  private:
    struct TaskEntry {
        TaskEntry(InterruptQueue& q, InterruptHandler& h,
                  MotorHardware& motor_iface)
            : queue{q}, handler{h}, iface{motor_iface} {}

        void operator()() {
            while (true) {
                auto move = MotorMoveMessage{};
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
        InterruptQueue& queue;
        InterruptHandler& handler;
        MotorHardware& iface;
    };

    TaskEntry task_entry;
    freertos_task::FreeRTOSTask<128, TaskEntry> task;
};

}  // namespace motor_interrupt_driver
