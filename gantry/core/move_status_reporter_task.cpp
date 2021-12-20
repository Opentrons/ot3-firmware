#include "gantry/core/move_status_reporter_task.hpp"

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

using namespace gantry_move_status_reporter_task;

static auto queue = freertos_message_queue::FreeRTOSMessageQueue<
    move_status_reporter_task::TaskMessage>{};

static auto task_entry = MoveStatusReporterTaskType{queue};

static auto task =
    freertos_task::FreeRTOSTask<512, MoveStatusReporterTaskType,
                                gantry_tasks::QueueClient>{task_entry};

auto gantry_move_status_reporter_task::start_task(
    gantry_tasks::QueueClient &queueClient) -> MoveStatusReporterTaskType & {
    task.start(5, "move status reporter task", &queueClient);
    return task_entry;
}