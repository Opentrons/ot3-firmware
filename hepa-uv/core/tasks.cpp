#include "hepa-uv/core/tasks.hpp"

#include "common/core/freertos_task.hpp"
#include "common/core/freertos_timer.hpp"
#include "hepa-uv/core/can_task.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "hepa-uv/firmware/utility_gpio.h"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic pop

static auto tasks = hepauv_tasks::AllTask{};
static auto queues = hepauv_tasks::QueueClient{can::ids::NodeId::hepa_uv};

static auto hepa_task_builder =
    freertos_task::TaskStarter<512, hepa_task::HepaTask>{};

/**
 * Start hepa_uv tasks.
 */
void hepauv_tasks::start_tasks(
    can::bus::CanBus& can_bus,
    gpio_drive_hardware::GpioDrivePins& gpio_drive_pins) {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);

    auto& hepa_task = hepa_task_builder.start(5, "hepa_fan", gpio_drive_pins);

    tasks.hepa_task_handler = &hepa_task;
    tasks.can_writer = &can_writer;

    queues.set_queue(&can_writer.get_queue());
    queues.hepa_queue = &hepa_task.get_queue();
}

hepauv_tasks::QueueClient::QueueClient(can::ids::NodeId this_fw)
    : can::message_writer::MessageWriter{this_fw} {}

void hepauv_tasks::QueueClient::send_interrupt_message(
    const hepa_task::TaskMessage& m) {
    hepa_queue->try_write(m);
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
