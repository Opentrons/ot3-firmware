#include "pipettes/core/move_group_task.hpp"

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"

using namespace pipettes_move_group_task;

static auto move_group_manager = move_group_task::MoveGroupType{};

static auto queue = freertos_message_queue::FreeRTOSMessageQueue<
    move_group_task::TaskMessage>{};

static auto task_entry = MoveGroupTaskType{queue, move_group_manager};

static auto task =
    freertos_task::FreeRTOSTask<512, MoveGroupTaskType,
                                pipettes_tasks::QueueClient,
                                pipettes_tasks::QueueClient>{task_entry};

auto pipettes_move_group_task::start_task(pipettes_tasks::QueueClient& client)
    -> MoveGroupTaskType& {
    task.start(8, "move group task", &client, &client);
    return task_entry;
}