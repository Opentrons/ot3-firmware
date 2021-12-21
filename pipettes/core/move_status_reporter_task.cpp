#include "pipettes/core/move_status_reporter_task.hpp"

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

using namespace pipettes_move_status_reporter_task;

static auto queue = freertos_message_queue::FreeRTOSMessageQueue<
    move_status_reporter_task::TaskMessage>{};

static auto task_entry = MoveStatusReporterTaskType{queue};

static auto task =
    freertos_task::FreeRTOSTask<512, MoveStatusReporterTaskType,
                                pipettes_tasks::QueueClient>{task_entry};

auto pipettes_move_status_reporter_task::start_task(
    pipettes_tasks::QueueClient &queueClient) -> MoveStatusReporterTaskType & {
    task.start(5, "move status reporter task", &queueClient);
    return task_entry;
}