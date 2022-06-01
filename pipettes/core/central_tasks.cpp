#include "pipettes/core/central_tasks.hpp"

#include "can/core/ids.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "pipettes/core/can_task.hpp"
#include "sensors/core/tasks/capacitive_sensor_task.hpp"
#include "sensors/core/tasks/environmental_sensor_task.hpp"
#include "sensors/core/tasks/pressure_sensor_task.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

static auto tasks = central_tasks::Tasks{};
static auto queue_client = central_tasks::QueueClient{};

void central_tasks::start_tasks(can::bus::CanBus& can_bus, can::ids::NodeId id) {
    queue_client.set_node_id(id);
    auto& queues = central_tasks::get_queues();
    auto& tasks = central_tasks::get_tasks();

    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus, id);

    tasks.can_writer = &can_writer;

    queues.set_queue(&can_writer.get_queue());
    queues.can_writer = &can_writer.get_queue();
}

central_tasks::QueueClient::QueueClient()
    // This gets overridden in start_tasks, needs to be static here since this
    // is free-store allocated
    : can::message_writer::MessageWriter{can::ids::NodeId::pipette_left} {}

auto central_tasks::get_tasks() -> Tasks& { return tasks; }

auto central_tasks::get_queues() -> QueueClient& { return queue_client; }
