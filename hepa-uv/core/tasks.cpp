#include "hepa-uv/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
#include "eeprom/core/dev_data.hpp"
#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/update_data_rev_task.hpp"
#include "hepa-uv/core/can_task.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "hepa-uv/firmware/hepa_control_hardware.hpp"
#include "hepa-uv/firmware/utility_gpio.h"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#pragma GCC diagnostic pop

static auto tasks = hepauv_tasks::AllTask{};
static auto queues = hepauv_tasks::QueueClient{can::ids::NodeId::hepa_uv};

static auto hepa_task_builder =
    freertos_task::TaskStarter<512, hepa_task::HepaTask>{};

static auto uv_task_builder =
    freertos_task::TaskStarter<512, uv_task::UVTask>{};

static auto led_control_task_builder =
    freertos_task::TaskStarter<512, led_control_task::LEDControlTask>{};

static auto eeprom_task_builder =
    freertos_task::TaskStarter<512, eeprom::task::EEPromTask>{};

static auto eeprom_data_rev_update_builder =
    freertos_task::TaskStarter<512, eeprom::data_rev_task::UpdateDataRevTask>{};

static auto i2c2_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto i2c2_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};

template <template <typename> typename QueueImpl>
using PollerWithTimer =
    i2c::tasks::I2CPollerTask<QueueImpl, freertos_timer::FreeRTOSTimer>;
static auto i2c2_poll_task_builder =
    freertos_task::TaskStarter<1024, PollerWithTimer>{};

static auto i2c2_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};

/**
 * Start hepa_uv tasks.
 */
void hepauv_tasks::start_tasks(
    can::bus::CanBus& can_bus,
    gpio_drive_hardware::GpioDrivePins& gpio_drive_pins,
    hepa_control_hardware::HepaControlHardware& hepa_hardware,
    led_control_hardware::LEDControlHardware& led_hardware,
    i2c::hardware::I2CBase& i2c2,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface) {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);

    auto& i2c2_task = i2c2_task_builder.start(5, "i2c2", i2c2);
    auto& i2c2_poller_task =
        i2c2_poll_task_builder.start(5, "i2c2 poller", i2c2_task_client);
    i2c2_poll_client.set_queue(&i2c2_poller_task.get_queue());
    i2c2_task_client.set_queue(&i2c2_task.get_queue());
    auto& eeprom_task = eeprom_task_builder.start(5, "eeprom", i2c2_task_client,
                                                  eeprom_hw_iface);

    auto& hepa_task = hepa_task_builder.start(5, "hepa_fan", gpio_drive_pins,
                                              hepa_hardware, queues);
    auto& uv_task =
        uv_task_builder.start(5, "uv_ballast", gpio_drive_pins, queues);
    auto& led_control_task =
        led_control_task_builder.start(5, "push_button_leds", led_hardware);

    tasks.can_writer = &can_writer;
    tasks.i2c2_task = &i2c2_task;
    tasks.i2c2_poller_task = &i2c2_poller_task;
    tasks.eeprom_task = &eeprom_task;
    tasks.hepa_task_handler = &hepa_task;
    tasks.uv_task_handler = &uv_task;
    tasks.led_control_task_handler = &led_control_task;

    queues.set_queue(&can_writer.get_queue());
    queues.i2c2_queue = &i2c2_task.get_queue();
    queues.i2c2_poller_queue = &i2c2_poller_task.get_queue();
    queues.eeprom_queue = &eeprom_task.get_queue();
    queues.hepa_queue = &hepa_task.get_queue();
    queues.uv_queue = &uv_task.get_queue();
    queues.led_control_queue = &led_control_task.get_queue();
}

hepauv_tasks::QueueClient::QueueClient(can::ids::NodeId this_fw)
    : can::message_writer::MessageWriter{this_fw} {}

void hepauv_tasks::QueueClient::send_eeprom_queue(
    const eeprom::task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}

void hepauv_tasks::QueueClient::send_hepa_message(
    const hepa_task::TaskMessage& m) {
    hepa_queue->try_write(m);
}

void hepauv_tasks::QueueClient::send_uv_message(const uv_task::TaskMessage& m) {
    uv_queue->try_write(m);
}

void hepauv_tasks::QueueClient::send_led_control_message(
    const led_control_task::TaskMessage& m) {
    led_control_queue->try_write(m);
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
