#include "pipettes/core/sensor_tasks.hpp"

#include "can/core/ids.hpp"
#include "common/core/freertos_task.hpp"

static auto tasks = sensor_tasks::Tasks{};
static auto queue_client = sensor_tasks::QueueClient{};

static auto eeprom_task_builder =
    freertos_task::TaskStarter<512, eeprom::task::EEPromTask>{};

static auto environment_sensor_task_builder =
    freertos_task::TaskStarter<512, sensors::tasks::EnvironmentSensorTask,
                               can::ids::SensorId>(can::ids::SensorId::S0);

static auto capacitive_sensor_task_builder_front =
    freertos_task::TaskStarter<512, sensors::tasks::CapacitiveSensorTask,
                               can::ids::SensorId>(can::ids::SensorId::S0);

static auto pressure_sensor_task_builder =
    freertos_task::TaskStarter<512, sensors::tasks::PressureSensorTask,
                               can::ids::SensorId>(can::ids::SensorId::S0);

void sensor_tasks::start_tasks(
    sensor_tasks::CanWriterTask& can_writer,
    sensor_tasks::I2CClient& i2c3_task_client,
    sensor_tasks::I2CClient& i2c1_task_client,
    sensor_tasks::I2CPollerClient& i2c1_poller_client,
    sensors::hardware::SensorHardwareBase& sensor_hardware, can::ids::NodeId id,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hardware) {
    queue_client.set_node_id(id);
    auto& queues = sensor_tasks::get_queues();
    auto& tasks = sensor_tasks::get_tasks();

    auto& eeprom_task = eeprom_task_builder.start(5, "eeprom", i2c3_task_client,
                                                  eeprom_hardware);
    auto& environment_sensor_task = environment_sensor_task_builder.start(
        5, "enviro sensor", i2c1_task_client, i2c1_poller_client, queues);
    auto& pressure_sensor_task = pressure_sensor_task_builder.start(
        5, "pressure sensor", i2c1_task_client, i2c1_poller_client, queues,
        sensor_hardware);
    auto& capacitive_sensor_task_front =
        capacitive_sensor_task_builder_front.start(
            5, "capacitive sensor s0", i2c1_task_client, i2c1_poller_client,
            sensor_hardware, queues);

    tasks.eeprom_task = &eeprom_task;
    tasks.environment_sensor_task = &environment_sensor_task;
    tasks.capacitive_sensor_task_front = &capacitive_sensor_task_front;
    tasks.pressure_sensor_task = &pressure_sensor_task;

    queues.set_queue(&can_writer.get_queue());
    queues.eeprom_queue = &eeprom_task.get_queue();
    queues.environment_sensor_queue = &environment_sensor_task.get_queue();
    queues.capacitive_sensor_queue_front =
        &capacitive_sensor_task_front.get_queue();
    queues.pressure_sensor_queue = &pressure_sensor_task.get_queue();
}

sensor_tasks::QueueClient::QueueClient()
    // This gets overridden in start_tasks, needs to be static here since this
    // is free-store allocated
    : can::message_writer::MessageWriter{can::ids::NodeId::pipette_left} {}

void sensor_tasks::QueueClient::send_eeprom_queue(
    const eeprom::task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}

void sensor_tasks::QueueClient::send_environment_sensor_queue(
    const sensors::utils::TaskMessage& m) {
    environment_sensor_queue->try_write(m);
}

void sensor_tasks::QueueClient::send_capacitive_sensor_queue_front(
    const sensors::utils::TaskMessage& m) {
    capacitive_sensor_queue_front->try_write(m);
}

void sensor_tasks::QueueClient::send_pressure_sensor_queue(
    const sensors::utils::TaskMessage& m) {
    pressure_sensor_queue->try_write(m);
}

auto sensor_tasks::get_tasks() -> Tasks& { return tasks; }

auto sensor_tasks::get_queues() -> QueueClient& { return queue_client; }