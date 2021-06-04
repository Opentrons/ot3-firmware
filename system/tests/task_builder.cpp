#include "test/task_builder.hpp"

TaskBuilder::TaskBuilder()
    : host_comms_queue("host comms"),
      host_comms_task(host_comms_queue),
      ui_queue("ui"),
      ui_task(ui_queue),
      task_aggregator{.comms = host_comms_task,
                      .ui = ui_task} {
    host_comms_task.provide_tasks(&task_aggregator);
    ui_task.provide_tasks(&task_aggregator);
    motor_task.provide_tasks(&task_aggregator);
    heater_task.provide_tasks(&task_aggregator);
}

auto TaskBuilder::build() -> std::shared_ptr<TaskBuilder> {
    return std::shared_ptr<TaskBuilder>(new TaskBuilder());
}
