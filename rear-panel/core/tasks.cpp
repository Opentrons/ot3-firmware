#include "rear-panel/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
#include "rear-panel/core/lights/animation_handler.hpp"
#include "rear-panel/core/tasks/light_control_update_timer.hpp"
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
                                .active_setting = SYNC_MCU_OUT_AS},
    .estop_in = gpio::PinConfig{.port = ESTOP_MCU_IN_PORT,
                                .pin = ESTOP_MCU_IN_PIN,
                                .active_setting = ESTOP_MCU_IN_AS},
    .estop_aux1_det = gpio::PinConfig{.port = ESTOP_DETECT_AUX1_MCU_PORT,
                                      .pin = ESTOP_DETECT_AUX1_MCU_PIN,
                                      .active_setting = ESTOP_DETECT_AUX_AS},
    .estop_aux2_det = gpio::PinConfig{.port = ESTOP_DETECT_AUX2_MCU_PORT,
                                      .pin = ESTOP_DETECT_AUX2_MCU_PIN,
                                      .active_setting = ESTOP_DETECT_AUX_AS},
    .door_open = gpio::PinConfig{.port = DOOR_OPEN_MCU_PORT,
                                 .pin = DOOR_OPEN_MCU_PIN,
                                 .active_setting = DOOR_OPEN_MCU_AS}};

static auto light_control_task_builder =
    freertos_task::TaskStarter<512, light_control_task::LightControlTask>{};

static auto system_task_builder =
    freertos_task::TaskStarter<512, system_task::SystemTask>{};

static auto light_control_timer = light_control_task::timer::LightControlTimer(
    light_control_task_builder.queue, light_control_task::DELAY_MS);

static auto hardware_task_builder =
    freertos_task::TaskStarter<512, hardware_task::HardwareTask>{};

static auto animation_handler = light_control_task::Animation();

/**
 * Start rear tasks.
 */
void rear_panel_tasks::start_tasks(
    light_control_task::LightControlInterface& light_hardware
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

    auto& light_task = light_control_task_builder.start(
        5, "lights", light_hardware, animation_handler);
    queues.light_control_queue = &light_task.get_queue();
    tasks.light_control_task = &light_task;
    light_control_timer.start();

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

    auto& hardwarectl_task =
        hardware_task_builder.start(5, "hardware", gpio_drive_pins);
    tasks.hardware_task = &hardwarectl_task;
    queues.hardware_queue = &hardwarectl_task.get_queue();
}

/**
 * Access to the tasks singleton
 * @return
 */
auto rear_panel_tasks::get_all_tasks() -> AllTask& { return tasks; }
