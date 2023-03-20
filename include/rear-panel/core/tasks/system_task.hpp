/*
 * Interface for the firmware-specific system tasks
 */
#pragma once
#include <tuple>

#include "FreeRTOS.h"
#include "common/core/message_queue.hpp"
#include "common/core/version.h"
#include "common/firmware/gpio.hpp"
#include "rear-panel/core/binary_parse.hpp"
#include "rear-panel/core/double_buffer.hpp"
#include "rear-panel/core/messages.hpp"
#include "rear-panel/core/tasks.hpp"
#include "rear-panel/firmware/gpio_drive_hardware.hpp"
#include "rear-panel/firmware/system_hardware.h"

namespace system_task {

using TaskMessage = rearpanel::messages::SystemTaskMessage;

class SystemMessageHandler {
  public:
    explicit SystemMessageHandler(
        gpio_drive_hardware::GpioDrivePins& drive_pins)
        : drive_pins{drive_pins} {}
    SystemMessageHandler(const SystemMessageHandler&) = delete;
    SystemMessageHandler(const SystemMessageHandler&&) = delete;
    auto operator=(const SystemMessageHandler&)
        -> SystemMessageHandler& = delete;
    auto operator=(const SystemMessageHandler&&)
        -> SystemMessageHandler&& = delete;
    ~SystemMessageHandler() = default;

    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    static void handle(std::monostate&) {}

    static void handle(rearpanel::messages::EnterBootloader&) {
        system_hardware_enter_bootloader();
        // above function does not return
    }
    void handle(const rearpanel::messages::EngageEstopRequest&) const {
        gpio::set(drive_pins.estop_out);
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(rearpanel::messages::Ack{});
    }
    void handle(const rearpanel::messages::ReleaseEstopRequest&) const {
        gpio::reset(drive_pins.estop_out);
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(rearpanel::messages::Ack{});
    }
    void handle(const rearpanel::messages::EngageSyncRequest&) const {
        gpio::set(drive_pins.sync_out);
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(rearpanel::messages::Ack{});
    }
    void handle(const rearpanel::messages::ReleaseSyncRequest&) const {
        gpio::reset(drive_pins.sync_out);
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(rearpanel::messages::Ack{});
    }
    void handle(const rearpanel::messages::DoorSwitchStateRequest&) const {
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(
            rearpanel::messages::DoorSwitchStateResponse{
                .open = gpio::is_set(drive_pins.door_open)});
    }
    gpio_drive_hardware::GpioDrivePins& drive_pins;
};

/**
 * The task entry point.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class SystemTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    SystemTask(QueueType& queue) : queue{queue} {}
    SystemTask(const SystemTask& c) = delete;
    SystemTask(const SystemTask&& c) = delete;
    auto operator=(const SystemTask& c) = delete;
    auto operator=(const SystemTask&& c) = delete;
    ~SystemTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(
        gpio_drive_hardware::GpioDrivePins* drive_pins) {
        auto handler = SystemMessageHandler{*drive_pins};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
};

}  // namespace system_task
