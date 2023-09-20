#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.h"
#include "motor-control/core/brushed_motor/brushed_motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"

namespace brushed_motor_interrupt_driver {
template <class StatusClient, class BrushedMotorHardware>
class BrushedMotorInterruptDriver {
    using InterruptQueue = freertos_message_queue::FreeRTOSMessageQueue<
        motor_messages::BrushedMove>;
    using InterruptHandler =
        brushed_motor_handler::BrushedMotorInterruptHandler<
            freertos_message_queue::FreeRTOSMessageQueue, StatusClient>;

  public:
    BrushedMotorInterruptDriver(InterruptQueue& q, InterruptHandler& h,
                                BrushedMotorHardware& iface)
        : task_entry{q, h, iface}, task(task_entry) {
        task.start(5, "sim_motor_isr");
    }

  private:
    struct TaskEntry {
        TaskEntry(InterruptQueue& q, InterruptHandler& h,
                  BrushedMotorHardware& motor_iface)
            : queue{q}, handler{h}, iface{motor_iface} {}

        void operator()() {
            while (true) {
                auto move = motor_messages::BrushedMove{};
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
                                handler.set_enc_idle_state(true);
                                LOG("Received Home Request, triggering limit "
                                    "switch\n");
                            }
                            if (move.stop_condition ==
                                motor_messages::MoveStopCondition::none) {
                                LOG("Got Grip request, triggering idle\n");
                                handler.set_enc_idle_state(true);
                            }
                            if (move.stop_condition ==
                                motor_messages::MoveStopCondition::
                                    encoder_position) {
                                LOG("Got move request, setting position\n");
                                iface.set_encoder_pulses(move.encoder_position);
                            }
                        }
                        handler.run_interrupt();

                    } while (iface.get_motor_state() ==
                             BrushedMotorState::FORCE_CONTROLLING);
                    LOG("Move completed. Stopping interrupt simulation..");
                }
            }
        }
        InterruptQueue& queue;
        InterruptHandler& handler;
        BrushedMotorHardware& iface;
    };

    TaskEntry task_entry;
    freertos_task::FreeRTOSTask<128, TaskEntry> task;
};
}  // namespace brushed_motor_interrupt_driver
