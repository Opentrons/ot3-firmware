#include "pipettes/core/sensor_tasks.hpp"

#include "can/core/ids.hpp"
#include "common/core/freertos_task.hpp"
#include "pipettes/core/pipette_type.h"

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

static auto usage_storage_task_builder =
    freertos_task::TaskStarter<512, usage_storage_task::UsageStorageTask>{};

void sensor_tasks::start_tasks(
    sensor_tasks::CanWriterTask& can_writer,
    sensor_tasks::I2CClient& i2c3_task_client,
    sensor_tasks::I2CPollerClient& i2c3_poller_client,
    sensor_tasks::I2CClient& i2c1_task_client,
    sensor_tasks::I2CPollerClient& i2c1_poller_client,
    sensors::hardware::SensorHardwareBase& sensor_hardware, can::ids::NodeId id,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hardware) {
    queue_client.set_node_id(id);
    auto& queues = sensor_tasks::get_queues();
    auto& tasks = sensor_tasks::get_tasks();

    auto& pressure_i2c_client = get_pipette_type() == EIGHT_CHANNEL
                                    ? i2c3_task_client
                                    : i2c1_task_client;
    auto& pressure_i2c_poller = get_pipette_type() == EIGHT_CHANNEL
                                    ? i2c3_poller_client
                                    : i2c1_poller_client;

    auto& eeprom_task = eeprom_task_builder.start(5, "eeprom", i2c3_task_client,
                                                  eeprom_hardware);
    auto& environment_sensor_task = environment_sensor_task_builder.start(
        5, "enviro sensor", i2c1_task_client, i2c1_poller_client, queues);
    auto& pressure_sensor_task = pressure_sensor_task_builder.start(
        5, "pressure sensor", pressure_i2c_client, pressure_i2c_poller, queues,
        sensor_hardware);
    auto& capacitive_sensor_task_front =
        capacitive_sensor_task_builder_front.start(
            5, "capacitive sensor s0", i2c1_task_client, i2c1_poller_client,
            sensor_hardware, queues);
    auto& usage_storage_task =
        usage_storage_task_builder.start(5, "usage storage", queues, queues);

    tasks.eeprom_task = &eeprom_task;
    tasks.environment_sensor_task = &environment_sensor_task;
    tasks.capacitive_sensor_task_front = &capacitive_sensor_task_front;
    tasks.pressure_sensor_task = &pressure_sensor_task;
    tasks.usage_storage_task = &usage_storage_task;

    queues.set_queue(&can_writer.get_queue());
    queues.eeprom_queue = &eeprom_task.get_queue();
    queues.environment_sensor_queue = &environment_sensor_task.get_queue();
    queues.capacitive_sensor_queue_front =
        &capacitive_sensor_task_front.get_queue();
    queues.pressure_sensor_queue = &pressure_sensor_task.get_queue();
    queues.usage_storage_queue = &usage_storage_task.get_queue();
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

void sensor_tasks::QueueClient::send_usage_storage_queue(
    const usage_storage_task::TaskMessage& m) {
    usage_storage_queue->try_write(m);
}

auto sensor_tasks::get_tasks() -> Tasks& { return tasks; }

auto sensor_tasks::get_queues() -> QueueClient& { return queue_client; }