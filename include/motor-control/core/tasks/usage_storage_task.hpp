#pragma once
#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "eeprom/core/dev_data.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace usage_storage_task {

// Not my favorite way to check this, but if we don't have access
// to vTaskDelay during host compilation so just dummy the function

static void _hardware_delay(uint ticks) {
#ifndef INC_TASK_H
    std::ignore = ticks;
#else
    vTaskDelay(ticks);
#endif
}

using TaskMessage = motor_control_task_messages::UsageStorageTaskMessage;

static constexpr uint16_t distance_data_usage_len = 8;

/**
 * The message queue message handler.
 */
template <can::message_writer_task::TaskClient CanClient,
          eeprom::task::TaskClient EEPromClient>
class UsageStorageTaskHandler : eeprom::accessor::ReadListener {
  public:
    UsageStorageTaskHandler(CanClient& can_client, EEPromClient& eeprom_client, eeprom::dev_data::DevDataTailAccessor<EEPromClient>& tail_accessor)
        : can_client{can_client},
          usage_data_accessor{eeprom_client, *this, accessor_backing, tail_accessor} {}
    UsageStorageTaskHandler(const UsageStorageTaskHandler& c) = delete;
    UsageStorageTaskHandler(const UsageStorageTaskHandler&& c) = delete;
    auto operator=(const UsageStorageTaskHandler& c) = delete;
    auto operator=(const UsageStorageTaskHandler&& c) = delete;
    ~UsageStorageTaskHandler() final = default;

    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->start_handle(m); }, message);
    }

    void read_complete(uint32_t message_index) final {
        std::ignore = message_index;
        std::visit([this](auto m) { this->finish_handle(m); }, buffered_task);
        ready_for_new_message = true;
    }

    auto ready() -> bool {
        return ready_for_new_message && usage_data_accessor.ready();
    }

  private:
    void start_handle(const std::monostate& m) { static_cast<void>(m); }

    void finish_handle(const std::monostate& m) { static_cast<void>(m); }

    void start_handle(const usage_messages::GetUsageRequest& m) {
        ready_for_new_message = false;
        buffered_task = m;
        _ensure_part(m.distance_usage_key, distance_data_usage_len);
        usage_data_accessor.get_data(m.distance_usage_key, 0);
    }

    void finish_handle(const usage_messages::GetUsageRequest& m) {
        uint64_t distance_usage = 0;
        std::ignore = bit_utils::bytes_to_int(
            accessor_backing.begin(),
            accessor_backing.begin() + distance_data_usage_len, distance_usage);
        can::messages::GetMotorUsageResponse resp = {
            .message_index = m.message_index,
            .distance_usage_um = distance_usage};
        can_client.send_can_message(can::ids::NodeId::host, resp);
    }

    void start_handle(const usage_messages::IncreaseDistanceUsage& m) {
        ready_for_new_message = false;
        buffered_task = m;
        _ensure_part(m.key, distance_data_usage_len);
        usage_data_accessor.get_data(m.key, 0);
    }

    void finish_handle(const usage_messages::IncreaseDistanceUsage& m) {
        uint64_t old_value = 0;
        std::ignore = bit_utils::bytes_to_int(
            accessor_backing.begin(),
            accessor_backing.begin() + distance_data_usage_len, old_value);
        old_value += m.distance_traveled_um;
        std::ignore = bit_utils::int_to_bytes(
            old_value, accessor_backing.begin(), accessor_backing.end());
        usage_data_accessor.write_data(m.key, distance_data_usage_len,
                                       accessor_backing);
    }

    void _ensure_part(uint16_t key, uint16_t len) {
        // If the partition has been created, make it and wait for
        // the partition table to update
        if (!usage_data_accessor.data_part_exists(key)) {
            usage_data_accessor.create_data_part(key, len);
            while (!usage_data_accessor.ready()) {
                _hardware_delay(10);
            }
        }
    }

    TaskMessage buffered_task = {};
    bool ready_for_new_message = true;
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
    template <can::message_writer_task::TaskClient CanClient,
              eeprom::task::TaskClient EEPromClient>
    [[noreturn]] void operator()(CanClient* can_client,
                                 EEPromClient* eeprom_client,
                                 eeprom::dev_data::DevDataTailAccessor<EEPromClient>* tail_accessor) {
        auto handler = UsageStorageTaskHandler{*can_client, *eeprom_client, *tail_accessor};
        TaskMessage message{};
        for (;;) {
            if (handler.ready()) {
                if (queue.try_read(&message, queue.max_delay)) {
                    handler.handle_message(message);
                }
            } else {
                // wait for the handler to be ready before sending the next
                // message
                _hardware_delay(10);
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
