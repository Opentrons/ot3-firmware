#include "hepa-uv/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
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

/**
 * Start hepa_uv tasks.
 */
void hepauv_tasks::start_tasks(
    can::bus::CanBus& can_bus,
    gpio_drive_hardware::GpioDrivePins& gpio_drive_pins,
    hepa_control_hardware::HepaControlHardware& hepa_hardware,
    led_control_hardware::LEDControlHardware& led_hardware) {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);

    // TODO: including led_hardware for testing, this should be a AssesorClient
    auto& hepa_task = hepa_task_builder.start(5, "hepa_fan", gpio_drive_pins,
                                              hepa_hardware, queues);
    auto& uv_task =
        uv_task_builder.start(5, "uv_ballast", gpio_drive_pins, queues);
    auto& led_control_task =
        led_control_task_builder.start(5, "push_button_leds", led_hardware);

    tasks.hepa_task_handler = &hepa_task;
    tasks.uv_task_handler = &uv_task;
    tasks.led_control_task_handler = &led_control_task;
    tasks.can_writer = &can_writer;

    queues.set_queue(&can_writer.get_queue());
    queues.hepa_queue = &hepa_task.get_queue();
    queues.uv_queue = &uv_task.get_queue();
    queues.led_control_queue = &led_control_task.get_queue();
}

hepauv_tasks::QueueClient::QueueClient(can::ids::NodeId this_fw)
    : can::message_writer::MessageWriter{this_fw} {}

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
