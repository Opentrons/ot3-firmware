/*
 * Interface for the firmware-specific system tasks
 */
#pragma once
#include <tuple>

#include "FreeRTOS.h"
#include "common/core/message_queue.hpp"
#include "common/core/version.h"
#include "common/firmware/gpio.hpp"
#include "eeprom/core/messages.hpp"
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
            rearpanel::messages::DoorSwitchStateInfo{
                .open = gpio::is_set(drive_pins.door_open)});
    }
    void handle(const rearpanel::messages::AuxPresentRequeset&) const {
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(
            rearpanel::messages::AuxPortDetectionChange{
                .aux1_present = gpio::is_set(drive_pins.aux1_present),
                .aux2_present = gpio::is_set(drive_pins.aux2_present)});
    }
    void handle(const rearpanel::messages::AuxIDRequest&) const {
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(rearpanel::messages::AuxIDResponse{
            .aux1_id_state = gpio::is_set(drive_pins.aux1_id),
            .aux2_id_state = gpio::is_set(drive_pins.aux2_id)});
    }
    void handle(const rearpanel::messages::EstopButtonPresentRequest&) const {
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(
            rearpanel::messages::EstopButtonDetectionChange{
                .aux1_present = gpio::is_set(drive_pins.estop_aux1_det),
                .aux2_present = gpio::is_set(drive_pins.estop_aux2_det)});
    }
    void handle(const rearpanel::messages::EstopStateRequest&) const {
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(
            rearpanel::messages::EstopStateChange{
                .engaged = gpio::is_set(drive_pins.estop_in)});
    }
    void handle(const rearpanel::messages::SyncStateRequest&) const {
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(
            rearpanel::messages::SyncStateResponse{
                .engaged = gpio::is_set(drive_pins.sync_out)});
    }
    void handle(rearpanel::messages::ReadEEPromRequest& msg) {
        auto read_req = eeprom::message::ReadEepromMessage{
            .message_index = 0,
            .memory_address = msg.data_address,
            .length = msg.data_length,
            .callback = read_complete,
            .callback_param = this};
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_eeprom_queue(read_req);
    }

    static void read_complete(const eeprom::message::EepromMessage& msg,
                              void*) {
        auto resp = rearpanel::messages::ReadEEPromResponse{
            .length = 12,
            .data_address = msg.memory_address,
            .data_length = msg.length,
        };
        std::copy_n(msg.data.begin(), msg.length, resp.data.begin());
        auto queue_client = queue_client::get_main_queues();
        queue_client.send_host_comms_queue(resp);
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
