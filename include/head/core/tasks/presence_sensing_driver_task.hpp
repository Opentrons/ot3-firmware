#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.hpp"
#include "head/core/adc.hpp"
#include "head/core/presence_sensing_driver.hpp"

namespace presence_sensing_driver_task {

using TaskMessage =
    std::variant<std::monostate,
                 can_messages::ReadPresenceSensingVoltageRequest,
                 can_messages::PushToolsDetectedNotification>;

/**
 * The handler of Presence Sensing messages
 */
template <message_writer_task::TaskClient CanClient>
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

    void push_notification(const TaskMessage& notification) {
        std::visit([this](auto m) { this->visit(m); }, notification);
    }

  private:
    void visit(std::monostate& m) {}

    void visit(can_messages::ReadPresenceSensingVoltageRequest& m) {
        auto voltage_read = driver.get_readings();

        can_messages::ReadPresenceSensingVoltageResponse resp{
            .z_motor = voltage_read.z_motor,
            .a_motor = voltage_read.a_motor,
            .gripper = voltage_read.gripper,
        };
        can_client.send_can_message(can_ids::NodeId::host, resp);
    }

    void visit(can_messages::PushToolsDetectedNotification& m) {
        auto at = presence_sensing_driver::PresenceSensingDriver::get_tool(
            driver.get_readings());

        can_messages::PushToolsDetectedNotification resp{
            .z_motor = static_cast<uint16_t>(at.z_motor),
            .a_motor = static_cast<uint16_t>(at.a_motor),
            .gripper = static_cast<uint16_t>(at.gripper),
        };

        can_client.send_can_message(can_ids::NodeId::host, resp);
    }

    presence_sensing_driver::PresenceSensingDriver& driver;
    CanClient& can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl,
          message_writer_task::TaskClient CanClient>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class PresenceSensingDriverTask {
  public:
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
    [[noreturn]] void operator()(
        presence_sensing_driver::PresenceSensingDriver* driver,
        CanClient* can_client) {
        auto current_tools =
            presence_sensing_driver::PresenceSensingDriver::get_tool(
                driver->get_readings());
        auto handler =
            PresenceSensingDriverMessageHandler{*driver, *can_client};
        TaskMessage message{};
        auto notification =
            TaskMessage(can_messages::PushToolsDetectedNotification());
        auto tools = presence_sensing_driver::PresenceSensingDriver::get_tool(
            driver->get_readings());
        for (;;) {
            tools = presence_sensing_driver::PresenceSensingDriver::get_tool(
                driver->get_readings());
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
            if (tools.z_motor != current_tools.z_motor ||
                tools.a_motor != current_tools.a_motor ||
                tools.gripper != current_tools.gripper) {
                handler.push_notification(notification);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

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