#include "can/core/ids.hpp"
#include "common/core/freertos_task.hpp"
#include "pipettes/core/head_sensor_tasks.hpp"

static auto tasks = head_sensor_tasks::Tasks{};
static auto queue_client = head_sensor_tasks::QueueClient{};

static auto eeprom_task_builder =
    freertos_task::TaskStarter<512, eeprom::task::EEPromTask>{};

void head_sensor_tasks::start_tasks(
    head_sensor_tasks::CanWriterTask& can_writer,
    head_sensor_tasks::I2CClient& i2c3_task_client,
    head_sensor_tasks::I2CPollerClient&,
    can::ids::NodeId id,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hardware) {
    queue_client.set_node_id(id);
    auto& queues = head_sensor_tasks::get_queues();
    auto& tasks = head_sensor_tasks::get_tasks();

    auto& eeprom_task = eeprom_task_builder.start(5, "eeprom", i2c3_task_client,
                                                  eeprom_hardware);

    tasks.eeprom_task = &eeprom_task;

    queues.set_queue(&can_writer.get_queue());
    queues.eeprom_queue = &eeprom_task.get_queue();
}

head_sensor_tasks::QueueClient::QueueClient()
// This gets overridden in start_tasks, needs to be static here since this
// is free-store allocated
    : can::message_writer::MessageWriter{can::ids::NodeId::pipette_left} {}

void head_sensor_tasks::QueueClient::send_eeprom_queue(
    const eeprom::task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}

auto head_sensor_tasks::get_tasks() -> Tasks& { return tasks; }

auto head_sensor_tasks::get_queues() -> QueueClient& { return queue_client; }