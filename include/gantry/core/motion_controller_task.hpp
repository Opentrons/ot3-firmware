#pragma once

#include "gantry/core/tasks.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motion_controller.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"

namespace gantry_motion_controller_task {

using MotionControllerType =
    motion_controller::MotionController<lms::BeltConfig>;

using MotionControllerTaskType =
    motion_controller_task::MotionControllerTask<lms::BeltConfig,
                                                 gantry_tasks::QueueClient>;

auto start_task(MotionControllerType& controller,
                gantry_tasks::QueueClient& client) -> MotionControllerTaskType&;
}  // namespace gantry_motion_controller_task