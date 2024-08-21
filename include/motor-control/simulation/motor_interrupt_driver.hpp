#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.h"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"

namespace motor_interrupt_driver {

template <class StatusClient, class DriverClient, typename MotorMoveMessage,
          class MotorHardware>
class MotorInterruptDriver {
    using InterruptQueue =
        freertos_message_queue::FreeRTOSMessageQueue<MotorMoveMessage>;
    using MotorPositionUpdateQueue =
        freertos_message_queue::FreeRTOSMessageQueue<
            can::messages::UpdateMotorPositionEstimationRequest>;
    using InterruptHandler = motor_handler::MotorInterruptHandler<
        freertos_message_queue::FreeRTOSMessageQueue, StatusClient,
        DriverClient, MotorMoveMessage, MotorHardware>;

  public:
    MotorInterruptDriver(InterruptQueue& q, InterruptHandler& h,
                         MotorHardware& iface, MotorPositionUpdateQueue& pq)
        : task_entry{q, h, iface, pq}, task(task_entry) {
        task.start(5, "sim_motor_isr");
    }

  private:
    struct TaskEntry {
        TaskEntry(InterruptQueue& q, InterruptHandler& h,
                  MotorHardware& motor_iface, MotorPositionUpdateQueue& pq)
            : queue{q}, handler{h}, iface{motor_iface}, position_queue{pq} {}

        void operator()() {
            while (true) {
                auto move = MotorMoveMessage{};
                if (queue.peek(&move, 0)) {
                    LOG("Enabling motor interrupt handler for group %d, seq "
                        "%d, duration %ld",
                        move.group_id, move.seq_id, move.duration);
                    do {
                        if (queue.peek(&move, 0)) {
                            if (move.stop_condition ==
                                static_cast<uint8_t>(
                                    motor_messages::MoveStopCondition::
                                        limit_switch)) {
                                iface.trigger_limit_switch();
                                LOG("Received Home Request, triggering limit "
                                    "switch\n");
                            }
                        }
                        handler.run_interrupt();

                    } while (handler.has_active_move());
                    LOG("Move completed. Stopping interrupt simulation..");
                } else if (position_queue.has_message()) {
                    LOG("Running motor interrupt to update motor position from "
                        "encoder");
                    do {
                        handler.run_interrupt();
                    } while (position_queue.has_message());
                }
                taskYIELD();
            }
        }
        InterruptQueue& queue;
        InterruptHandler& handler;
        MotorHardware& iface;
        MotorPositionUpdateQueue& position_queue;
    };

    TaskEntry task_entry;
    freertos_task::FreeRTOSTask<128, TaskEntry> task;
};

}  // namespace motor_interrupt_driver
