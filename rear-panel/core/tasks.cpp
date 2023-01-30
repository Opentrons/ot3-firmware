#include "rear-panel/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
#include "rear-panel/firmware/freertos_comms_task.hpp"

static auto tasks = rear_panel_tasks::AllTask{};

static auto queues = rear_panel_tasks::QueueClient{};

//TODO(ryan): CORETASKS compile in and process the other basic tasks
/*
static auto eeprom_task_builder =
    freertos_task::TaskStarter<512, eeprom::task::EEPromTask>{};

static auto i2c3_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();
static auto i2c3_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};

template <template <typename> typename QueueImpl>
using PollerWithTimer =
    i2c::tasks::I2CPollerTask<QueueImpl, freertos_timer::FreeRTOSTimer>;
static auto i2c3_poll_task_builder =
    freertos_task::TaskStarter<1024, PollerWithTimer>{};

static auto i2c3_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};
*/

/**
 * Start rear tasks.
 */
void rear_panel_tasks::start_tasks(
    // i2c::hardware::I2CBase& i2c3,
    // eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface,
) {
    //TODO(ryan): CORETASKS compile in and process the other basic tasks
    /*
    auto& i2c3_task = i2c3_task_builder.start(5, "i2c3", i2c3);
    i2c3_task_client.set_queue(&i2c3_task.get_queue());

    auto& i2c3_poller_task =
        i2c3_poll_task_builder.start(5, "i2c3 poller", i2c3_task_client);
    i2c3_poll_client.set_queue(&i2c3_poller_task.get_queue());

    auto& eeprom_task = eeprom_task_builder.start(5, "eeprom", i2c3_task_client,
                                                  eeprom_hw_iface);
    */
    auto comms = host_comms_control_task::start();
    // tasks.i2c3_task = &i2c3_task;
    // tasks.i2c3_poller_task = &i2c3_poller_task;
    // tasks.eeprom_task = &eeprom_task;
    tasks.host_comms_task = comms.task;

    // queues.i2c3_queue = &i2c3_task.get_queue();
    // queues.i2c3_poller_queue = &i2c3_poller_task.get_queue();
    // queues.eeprom_queue = &eeprom_task.get_queue();
    queues.host_comms_queue = &comms.task->get_queue();
}

//TODO(ryan): CORETASKS compile in and process the other basic tasks
/*
void rear_tasks::QueueClient::send_eeprom_queue(
    const eeprom::task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}
*/

void rear_panel_tasks::QueueClient::send_host_comms_queue(
    const host_comms_task::TaskMessage& m) {
    host_comms_queue->try_write(m);
}

/**
 * Access to the tasks singleton
 * @return
 */
auto rear_panel_tasks::get_all_tasks() -> AllTask& { return tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto rear_panel_tasks::get_main_queues() -> QueueClient& { return queues; }
