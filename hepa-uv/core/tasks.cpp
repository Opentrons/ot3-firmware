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

static auto gpio_drive_pins = gpio_drive_hardware::GpioDrivePins{
    .door_open =
        gpio::PinConfig{
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = DOOR_OPEN_MCU_PORT,
            .pin = DOOR_OPEN_MCU_PIN,
            .active_setting = DOOR_OPEN_MCU_AS},
    .reed_switch =
        gpio::PinConfig{
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = REED_SW_MCU_PORT,
            .pin = REED_SW_MCU_PIN,
            .active_setting = REED_SW_MCU_AS},
    .hepa_push_button =
        gpio::PinConfig{
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = HEPA_NO_MCU_PORT,
            .pin = HEPA_NO_MCU_PIN,
        },
    .uv_push_button =
        gpio::PinConfig{
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = UV_NO_MCU_PORT,
            .pin = UV_NO_MCU_PIN,
        },
    .hepa_on_off =
        gpio::PinConfig{
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = HEPA_ON_OFF_PORT,
            .pin = HEPA_ON_OFF_PIN,
            .active_setting = HEPA_ON_OFF_AS},
    .uv_on_off = gpio::PinConfig{
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = UV_ON_OFF_MCU_PORT,
        .pin = UV_ON_OFF_MCU_PIN,
        .active_setting = UV_ON_OFF_AS}};

static auto hepa_task_builder =
    freertos_task::TaskStarter<512, hepa_task::HepaTask>{};

/**
 * Start hepa_uv tasks.
 */
void hepauv_tasks::start_tasks(can::bus::CanBus& can_bus) {
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
