#include "pipettes/core/motion_controller_task.hpp"

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"

using namespace pipettes_motion_controller_task;

static auto queue = freertos_message_queue::FreeRTOSMessageQueue<
    motion_controller_task::TaskMessage>{};

static auto task_entry = MotionControllerTaskType{queue};
static auto task =
    freertos_task::FreeRTOSTask<512, MotionControllerTaskType,
                                MotionControllerType,
                                pipettes_tasks::QueueClient>{task_entry};

auto pipettes_motion_controller_task::start_task(
    MotionControllerType &controller, pipettes_tasks::QueueClient &client)
    -> MotionControllerTaskType & {
    task.start(5, "motion controller task", &controller, &client);
    return task_entry;
}