#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace task_holder {

using namespace freertos_message_queue;
using namespace motor_control_task_messages;

struct Client {
    void send_motion_controller_queue(const MotionControlTaskMessage&) {}
    FreeRTOSMessageQueue<MotionControlTaskMessage>* motion_controller_queue;
    FreeRTOSMessageQueue<MoveGroupTaskMessage>* move_group_queue;
    FreeRTOSMessageQueue<MotorDriverTaskMessage>* motor_driver_queue;
    FreeRTOSMessageQueue<MoveStatusReporterTaskMessage>*
        move_status_reporter_queue;
};

}  // namespace task_holder
