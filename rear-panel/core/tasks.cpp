#include "rear-panel/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
#include "rear-panel/firmware/freertos_comms_task.hpp"
#include "rear-panel/firmware/gpio_drive_hardware.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "rear-panel/firmware/utility_gpio.h"
#pragma GCC diagnostic pop

static auto tasks = rear_panel_tasks::AllTask{};

// TODO(ryan): CORETASKS compile in and process the other basic tasks
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

static auto gpio_drive_pins = gpio_drive_hardware::GpioDrivePins{
    .estop_out = gpio::PinConfig{.port = ESTOP_MCU_OUT_PORT,
                                 .pin = ESTOP_MCU_OUT_PIN,
                                 .active_setting = ESTOP_MCU_OUT_AS},
    .sync_out = gpio::PinConfig{.port = SYNC_MCU_OUT_PORT,
                                .pin = SYNC_MCU_OUT_PIN,
                                .active_setting = SYNC_MCU_OUT_AS}};

static auto system_task_builder =
    freertos_task::TaskStarter<512, system_task::SystemTask>{};
/**
 * Start rear tasks.
 */
void rear_panel_tasks::start_tasks(
    // i2c::hardware::I2CBase& i2c3,
    // eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface,
) {
    auto& queues = queue_client::get_main_queues();
    // TODO(ryan): CORETASKS compile in and process the other basic tasks
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
    queues.host_comms_queue = &comms.task->get_queue();
    tasks.host_comms_task = comms.task;

    // tasks.i2c3_task = &i2c3_task;
    // tasks.i2c3_poller_task = &i2c3_poller_task;
    // tasks.eeprom_task = &eeprom_task;

    // queues.i2c3_queue = &i2c3_task.get_queue();
    // queues.i2c3_poller_queue = &i2c3_poller_task.get_queue();
    // queues.eeprom_queue = &eeprom_task.get_queue();
    auto& systemctl_task =
        system_task_builder.start(5, "system", gpio_drive_pins);
    tasks.system_task = &systemctl_task;
    queues.system_queue = &systemctl_task.get_queue();
}

/**
 * Access to the tasks singleton
 * @return
 */
auto rear_panel_tasks::get_all_tasks() -> AllTask& { return tasks; }
