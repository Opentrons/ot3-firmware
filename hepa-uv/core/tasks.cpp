#include "hepa-uv/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
#include "eeprom/core/dev_data.hpp"
#include "hepa-uv/core/can_task.hpp"
#include "hepa-uv/firmware/eeprom_keys.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "hepa-uv/firmware/utility_gpio.h"
#pragma GCC diagnostic pop

static auto tasks = hepauv_tasks::AllTask{};
static auto queues = hepauv_tasks::QueueClient{can::ids::NodeId::hepa_filter};

static auto eeprom_task_builder =
    freertos_task::TaskStarter<512, eeprom::task::EEPromTask>{};

static auto i2c2_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();
static auto i2c3_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto i2c2_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};
static auto i2c3_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};

template <template <typename> typename QueueImpl>
using PollerWithTimer =
    i2c::tasks::I2CPollerTask<QueueImpl, freertos_timer::FreeRTOSTimer>;
static auto i2c2_poll_task_builder =
    freertos_task::TaskStarter<1024, PollerWithTimer>{};
static auto i2c3_poll_task_builder =
    freertos_task::TaskStarter<1024, PollerWithTimer>{};

static auto i2c2_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};
static auto i2c3_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};

static auto tail_accessor = eeprom::dev_data::DevDataTailAccessor{queues};
static auto eeprom_data_rev_update_builder =
    freertos_task::TaskStarter<512, eeprom::data_rev_task::UpdateDataRevTask>{};

// PushButtonBlink Test Task
static auto pushbutton_task_builder =
    freertos_task::TaskStarter<512, pushbutton_task::PushButtonTask>{};

static auto gpio_drive_pins = gpio_drive_hardware::GpioDrivePins {
    .push_button_led = gpio::PinConfig {
        .port = LED_DRIVE_PORT,
        .pin = LED_DRIVE_PIN,
        .active_setting = GPIO_PIN_SET
    },
    .push_button = gpio::PinConfig {
        .port = HEPA_NO_MCU_PORT,
        .pin = HEPA_NO_MCU_PIN,
        .active_setting = GPIO_PIN_SET
    },
    .reed_switch = gpio::PinConfig {
        .port = REED_SW_MCU_PORT,
        .pin = REED_SW_MCU_PIN,
        .active_setting = GPIO_PIN_SET
    },
    .door_open = gpio::PinConfig {
        .port = DOOR_OPEN_MCU_PORT,
        .pin = DOOR_OPEN_MCU_PIN,
        .active_setting = GPIO_PIN_SET
    }
};


/**
 * Start gripper tasks.
 */
void hepauv_tasks::start_tasks(
    can::bus::CanBus& can_bus,
    i2c::hardware::I2CBase& i2c2, i2c::hardware::I2CBase& i2c3,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface) {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);
    tasks.can_writer = &can_writer;
    queues.set_queue(&can_writer.get_queue());

    auto& i2c2_task = i2c2_task_builder.start(5, "i2c2", i2c2);
    i2c2_task_client.set_queue(&i2c2_task.get_queue());
    auto& i2c3_task = i2c3_task_builder.start(5, "i2c3", i2c3);
    i2c3_task_client.set_queue(&i2c3_task.get_queue());

    auto& i2c2_poller_task =
        i2c2_poll_task_builder.start(5, "i2c2 poller", i2c2_task_client);
    i2c2_poll_client.set_queue(&i2c2_poller_task.get_queue());
    auto& i2c3_poller_task =
        i2c3_poll_task_builder.start(5, "i2c3 poller", i2c3_task_client);
    i2c3_poll_client.set_queue(&i2c3_poller_task.get_queue());

    auto& eeprom_task = eeprom_task_builder.start(5, "eeprom", i2c3_task_client,
                                                  eeprom_hw_iface);
#if PCBA_PRIMARY_REVISION != 'b'
    auto& eeprom_data_rev_update_task = eeprom_data_rev_update_builder.start(
        5, "data_rev_update", queues, tail_accessor, table_updater);
#endif

    tasks.i2c2_task = &i2c2_task;
    tasks.i2c3_task = &i2c3_task;
    tasks.i2c2_poller_task = &i2c2_poller_task;
    tasks.i2c3_poller_task = &i2c3_poller_task;
    tasks.eeprom_task = &eeprom_task;
#if PCBA_PRIMARY_REVISION != 'b'
    tasks.update_data_rev_task = &eeprom_data_rev_update_task;
#endif

    queues.i2c2_queue = &i2c2_task.get_queue();
    queues.i2c3_queue = &i2c3_task.get_queue();
    queues.i2c2_poller_queue = &i2c2_poller_task.get_queue();
    queues.i2c3_poller_queue = &i2c3_poller_task.get_queue();
    queues.eeprom_queue = &eeprom_task.get_queue();

    // PushButtonBlink Test Task
    auto& pushbutton_task =
        pushbutton_task_builder.start(5, "pushbutton", gpio_drive_pins);
    tasks.pushbutton_task = &pushbutton_task;
}

hepauv_tasks::QueueClient::QueueClient(can::ids::NodeId this_fw)
    : can::message_writer::MessageWriter{this_fw} {}

void hepauv_tasks::QueueClient::send_eeprom_queue(
    const eeprom::task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}

/**
 * Access to the tasks singleton
 * @return
 */
auto hepauv_tasks::get_all_tasks() -> AllTask& { return tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto hepauv_tasks::get_main_queues() -> QueueClient& { return queues; }