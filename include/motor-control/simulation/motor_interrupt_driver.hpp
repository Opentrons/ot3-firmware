#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace motor_interrupt_driver {

class MotorInterruptDriver {
  public:
    MotorInterruptDriver(
        freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>& q,
        motor_handler::MotorInterruptHandler<
            freertos_message_queue::FreeRTOSMessageQueue>& h)
        : task_entry{q, h}, task(task_entry) {
        task.start(nullptr, 5, "motor interrupt driver");
    }

  private:
    struct TaskEntry {
        TaskEntry(freertos_message_queue::FreeRTOSMessageQueue<
                      motor_messages::Move>& q,
                  motor_handler::MotorInterruptHandler<
                      freertos_message_queue::FreeRTOSMessageQueue>& h)
            : queue{q}, handler{h} {}

        void operator()(void*) {
            while (true) {
                auto move = motor_messages::Move{};
                LOG("Waiting for next move.\n");
                if (queue.peek(&move, portMAX_DELAY)) {
                    LOG("Enabling motor interrupt handler for group %d, seq "
                        "%d, duration %d\n",
                        move.group_id, move.seq_id, move.duration);
                    do {
                        handler.run_interrupt();
                    } while (handler.has_active_move);
                }
            }
        }
        freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>&
            queue;
        motor_handler::MotorInterruptHandler<
            freertos_message_queue::FreeRTOSMessageQueue>& handler;
    };

    TaskEntry task_entry;
    freertos_task::FreeRTOSTask<128, TaskEntry, void> task;
};

}  // namespace motor_interrupt_driver