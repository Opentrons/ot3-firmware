#pragma once

#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motion_controller.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "pipettes/core/tasks.hpp"

namespace pipettes_motion_controller_task {

using MotionControllerType =
    motion_controller::MotionController<lms::LeadScrewConfig>;

using MotionControllerTaskType = motion_controller_task::MotionControllerTask<
    freertos_message_queue::FreeRTOSMessageQueue, lms::LeadScrewConfig,
    pipettes_tasks::QueueClient>;

auto start_task(MotionControllerType& controller,
                pipettes_tasks::QueueClient& client)
    -> MotionControllerTaskType&;
}  // namespace pipettes_motion_controller_task