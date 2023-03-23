#pragma once
#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "eeprom/core/dev_data.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace usage_storage_task {

using TaskMessage = motor_control_task_messages::UsageStorageTaskMessage;

/**
 * The message queue message handler.
 */
template <can::message_writer_task::TaskClient CanClient,
          eeprom::task::TaskClient EEPromClient>
class UsageStorageTaskHandler : eeprom::accessor::ReadListener {
  public:
    UsageStorageTaskHandler(CanClient& can_client, EEPromClient& eeprom_client)
        : can_client{can_client},
          usage_data_accessor{eeprom_client, *this, accessor_backing} {}
    UsageStorageTaskHandler(const UsageStorageTaskHandler& c) = delete;
    UsageStorageTaskHandler(const UsageStorageTaskHandler&& c) = delete;
    auto operator=(const UsageStorageTaskHandler& c) = delete;
    auto operator=(const UsageStorageTaskHandler&& c) = delete;
    ~UsageStorageTaskHandler() = default;

    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

    void read_complete(uint32_t message_index) { std::ignore = message_index; }

  private:
    void handle(std::monostate m) { static_cast<void>(m); }

    void handle(usage_messages::IncreaseStepperDistance& m) {
        std::ignore = std::abs(m.distance_traveled_um);
    }

    CanClient& can_client;
    eeprom::dev_data::DataBufferType<64> accessor_backing =
        eeprom::dev_data::DataBufferType<64>{};
    eeprom::dev_data::DevDataAccessor<EEPromClient> usage_data_accessor;
};

/**
 * The task entry point.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class UsageStorageTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    UsageStorageTask(QueueType& queue) : queue{queue} {}
    UsageStorageTask(const UsageStorageTask& c) = delete;
    UsageStorageTask(const UsageStorageTask&& c) = delete;
    auto operator=(const UsageStorageTask& c) = delete;
    auto operator=(const UsageStorageTask&& c) = delete;
    ~UsageStorageTask() = default;

    /**
     * Task entry point.
     */
    template <eeprom::task::TaskClient EEPromClient,
              can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(CanClient* can_client,
                                 EEPromClient* eeprom_client) {
        auto handler = UsageStorageTaskHandler{*can_client, *eeprom_client};
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

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_usage_storage_queue(m)};
};

}  // namespace usage_storage_task
