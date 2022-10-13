#pragma once

#include <tuple>
#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "head/core/adc.hpp"
#include "head/core/attached_tools.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/core/tasks/messages.hpp"

namespace presence_sensing_driver_task {

using TaskMessage = presence_sensing_driver_task_messages::TaskMessage;

/**
 * The handler of Presence Sensing messages
 */
template <can::message_writer_task::TaskClient CanClient>
class PresenceSensingDriverMessageHandler {
  public:
    PresenceSensingDriverMessageHandler(
        presence_sensing_driver::PresenceSensingDriver& driver,
        CanClient& can_client)
        : driver{driver}, can_client{can_client} {}
    PresenceSensingDriverMessageHandler(
        const PresenceSensingDriverMessageHandler& c) = delete;
    PresenceSensingDriverMessageHandler(
        const PresenceSensingDriverMessageHandler&& c) = delete;
    auto operator=(const PresenceSensingDriverMessageHandler& c) = delete;
    auto operator=(const PresenceSensingDriverMessageHandler&& c) = delete;
    ~PresenceSensingDriverMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->visit(m); }, message);
    }

  private:
    void visit(std::monostate&) {}

    void visit(can::messages::ReadPresenceSensingVoltageRequest&) {
        auto voltage_read = driver.get_readings();

        LOG("Received read presence sensing voltage request: z=%d, a=%d, "
            "gripper=%d",
            voltage_read.z_motor, voltage_read.a_motor, voltage_read.gripper);

        can::messages::ReadPresenceSensingVoltageResponse resp{
            .z_motor = voltage_read.z_motor,
            .a_motor = voltage_read.a_motor,
            .gripper = voltage_read.gripper,
        };
        can_client.send_can_message(can::ids::NodeId::host, resp);
    }

    void visit(can::messages::AttachedToolsRequest&) {
        LOG("Received attached tools request");
        auto tools = driver.update_tools();
        auto new_tools = tools.second;
        can_client.send_can_message(
            can::ids::NodeId::host,
            can::messages::PushToolsDetectedNotification{
                .z_motor = (new_tools.z_motor),
                .a_motor = (new_tools.a_motor),
                .gripper = (new_tools.gripper),
            });
    }

    void visit(presence_sensing_driver_task_messages::CheckForToolChange&) {
        attached_tools::AttachedTools new_tools;
        auto tool_update = driver.update_tools();
        bool new_tool_detected = tool_update.first;
        new_tools = tool_update.second;
        if (new_tool_detected) {
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::PushToolsDetectedNotification{
                    .z_motor = (new_tools.z_motor),
                    .a_motor = (new_tools.a_motor),
                    .gripper = (new_tools.gripper),
                });
        }
    }

    presence_sensing_driver::PresenceSensingDriver& driver;
    CanClient& can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class PresenceSensingDriverTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    PresenceSensingDriverTask(QueueType& queue) : queue{queue} {}
    PresenceSensingDriverTask(const PresenceSensingDriverTask& c) = delete;
    PresenceSensingDriverTask(const PresenceSensingDriverTask&& c) = delete;
    auto operator=(const PresenceSensingDriverTask& c) = delete;
    auto operator=(const PresenceSensingDriverTask&& c) = delete;
    ~PresenceSensingDriverTask() = default;

    /**
     * Task entry point.
     */
    template <can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(
        presence_sensing_driver::PresenceSensingDriver* driver,
        CanClient* can_client) {
        auto handler =
            PresenceSensingDriverMessageHandler{*driver, *can_client};
        TaskMessage message{};

        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

    void notifier_callback() {
        auto msg = TaskMessage(
            presence_sensing_driver_task_messages::CheckForToolChange());
        this->queue.try_write(msg);
    }

  private:
    QueueType& queue;
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_presence_sensing_driver_queue(m)};
};

}  // namespace presence_sensing_driver_task
