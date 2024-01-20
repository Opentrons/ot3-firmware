#include "hepa-uv/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
#include "hepa-uv/core/can_task.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
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

static auto light_control_task_builder =
    freertos_task::TaskStarter<512, light_control_task::LightControlTask>{};

/**
 * Start hepa_uv tasks.
 */
void hepauv_tasks::start_tasks(
    can::bus::CanBus& can_bus,
    gpio_drive_hardware::GpioDrivePins& gpio_drive_pins,
    light_control_hardware::LightControlHardware& led_hardware) {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);

    // TODO: including led_hardware for testing, this should be a AssesorClient
    auto& hepa_task = hepa_task_builder.start(5, "hepa_fan", gpio_drive_pins, led_hardware);
    auto& uv_task = uv_task_builder.start(5, "uv_ballast", gpio_drive_pins, led_hardware);
    auto& light_control_task = light_control_task_builder.start(5, "push_button_leds", led_hardware);

    tasks.hepa_task_handler = &hepa_task;
    tasks.uv_task_handler = &uv_task;
    tasks.light_control_task_handler = &light_control_task;
    tasks.can_writer = &can_writer;

    queues.set_queue(&can_writer.get_queue());
    queues.hepa_queue = &hepa_task.get_queue();
    queues.uv_queue = &uv_task.get_queue();
    queues.light_control_queue = &light_control_task.get_queue();
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

void hepauv_tasks::QueueClient::send_light_control_message(const light_control_task::TaskMessage& m) {
    light_control_queue->try_write(m);
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
