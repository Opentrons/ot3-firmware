#pragma once
#include <limits>
#include <type_traits>
#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/hardware_delay.hpp"
#include "common/core/logging.h"
#include "eeprom/core/dev_data.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace usage_storage_task {

using namespace usage_messages;

using TaskMessage = motor_control_task_messages::UsageStorageTaskMessage;

static constexpr uint16_t distance_data_usage_len = 8;
static constexpr uint16_t force_time_data_usage_len = 4;
static constexpr uint16_t error_count_usage_len = 4;

template <typename NUM_T>
requires std::is_integral_v<NUM_T> && std::is_unsigned_v<NUM_T>
[[nodiscard]] auto check_for_default_val(NUM_T val, size_t len = 0) -> NUM_T {
    // in most cases we want to use the compare to the actual max value,
    // but during the processing of GetUsageRequest the NUM_T is always uint64_t
    // so this lets us change shift over the MAX to the correct number of bytes
    // to compare, the actual value is in the top bytes
    NUM_T cmp = std::numeric_limits<NUM_T>::max()
                << ((sizeof(NUM_T) - len) * 8);
    if (val == cmp) {
        // old_value must be an uninitialized data field so set it to 0
        val = 0;
    }
    return val;
}
/**
 * The message queue message handler.
 */
template <can::message_writer_task::TaskClient CanClient,
          eeprom::task::TaskClient EEPromClient>
class UsageStorageTaskHandler : eeprom::accessor::ReadListener {
  public:
    UsageStorageTaskHandler(
        CanClient& can_client, EEPromClient& eeprom_client,
        eeprom::dev_data::DevDataTailAccessor<EEPromClient>& tail_accessor)
        : can_client{can_client},
          usage_data_accessor{eeprom_client, *this, accessor_backing,
                              tail_accessor} {}
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
    }

    auto ready() -> bool {
        return ready_for_new_message && usage_data_accessor.read_write_ready();
    }

  private:
    void start_handle(const std::monostate& m) { static_cast<void>(m); }

    void finish_handle(const std::monostate& m) {
        static_cast<void>(m);
        ready_for_new_message = true;
    }

    void start_next_read(const GetUsageRequest& m) {
        if (m.usage_conf.num_keys != response.num_keys) {
            std::fill(accessor_backing.begin(), accessor_backing.end(), 0x00);
            usage_data_accessor.get_data(
                m.usage_conf.usage_requests[response.num_keys].eeprom_key,
                m.message_index);
        } else {
            can_client.send_can_message(can::ids::NodeId::host, response);
            ready_for_new_message = true;
            buffered_task = TaskMessage{};
        }
    }

    void start_handle(const GetUsageRequest& m) {
        ready_for_new_message = false;
        buffered_task = TaskMessage{m};
        response.message_index = m.message_index;
        response.num_keys = 0;
        start_next_read(m);
    }

    void finish_handle(const GetUsageRequest& m) {
        // parse the next usage value and construct response field
        uint64_t read_value = 0;
        std::ignore = bit_utils::bytes_to_int(
            accessor_backing.begin(), accessor_backing.end(), read_value);
        auto next_klv = can::messages::GetMotorUsageResponse::UsageValueField{
            .key = m.usage_conf.usage_requests[response.num_keys].type_key,
            .len = m.usage_conf.usage_requests[response.num_keys].length,
            .value = check_for_default_val(
                read_value,
                m.usage_conf.usage_requests[response.num_keys].length)};
        // add the next value to the response, and increment num_keys
        response.values[response.num_keys] = next_klv;
        response.num_keys += 1;
        // remove this request from the struct, save it to buffered task and
        // continue
        start_next_read(m);
        // the start_next_read task will handle sending the response if there is
        // no more requests
    }

    void start_handle(const IncreaseForceTimeUsage& m) {
        ready_for_new_message = false;
        buffered_task = TaskMessage{m};
        usage_data_accessor.get_data(m.key, 0);
    }

    void finish_handle(const IncreaseForceTimeUsage& m) {
        uint32_t old_value = 0;
        std::ignore = bit_utils::bytes_to_int(
            accessor_backing.begin(),
            accessor_backing.begin() + force_time_data_usage_len, old_value);
        old_value = check_for_default_val(old_value);
        old_value += m.seconds;
        std::ignore = bit_utils::int_to_bytes(
            old_value, accessor_backing.begin(), accessor_backing.end());
        usage_data_accessor.write_data(m.key, force_time_data_usage_len,
                                       accessor_backing);
        ready_for_new_message = true;
        buffered_task = TaskMessage{};
    }

    void start_handle(const IncreaseDistanceUsage& m) {
        ready_for_new_message = false;
        buffered_task = TaskMessage{m};
        usage_data_accessor.get_data(m.key, 0);
    }

    void finish_handle(const IncreaseDistanceUsage& m) {
        uint64_t old_value = 0;
        std::ignore = bit_utils::bytes_to_int(
            accessor_backing.begin(),
            accessor_backing.begin() + distance_data_usage_len, old_value);
        old_value = check_for_default_val(old_value);
        old_value += m.distance_traveled_um;
        std::ignore = bit_utils::int_to_bytes(
            old_value, accessor_backing.begin(), accessor_backing.end());
        usage_data_accessor.write_data(m.key, distance_data_usage_len,
                                       accessor_backing);
        ready_for_new_message = true;
        buffered_task = TaskMessage{};
    }

    void start_handle(const IncreaseErrorCount& m) {
        ready_for_new_message = false;
        buffered_task = TaskMessage{m};
        usage_data_accessor.get_data(m.key, 0);
    }

    void finish_handle(const IncreaseErrorCount& m) {
        uint32_t old_value = 0;
        std::ignore = bit_utils::bytes_to_int(
            accessor_backing.begin(),
            accessor_backing.begin() + error_count_usage_len, old_value);
        old_value = check_for_default_val(old_value);
        old_value += 1;
        std::ignore = bit_utils::int_to_bytes(
            old_value, accessor_backing.begin(), accessor_backing.end());
        usage_data_accessor.write_data(m.key, error_count_usage_len,
                                       accessor_backing);
        ready_for_new_message = true;
        buffered_task = TaskMessage{};
    }

    TaskMessage buffered_task = {};
    can::messages::GetMotorUsageResponse response = {};
    bool ready_for_new_message = true;
    CanClient& can_client;
    eeprom::dev_data::DataBufferType<8> accessor_backing =
        eeprom::dev_data::DataBufferType<8>{};
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
    [[noreturn]] void operator()(
        CanClient* can_client, EEPromClient* eeprom_client,
        eeprom::dev_data::DevDataTailAccessor<EEPromClient>* tail_accessor) {
        auto handler = UsageStorageTaskHandler{*can_client, *eeprom_client,
                                               *tail_accessor};
        TaskMessage message{};
        for (;;) {
            if (handler.ready()) {
                if (queue.try_read(&message, queue.max_delay)) {
                    handler.handle_message(message);
                }
            } else {
                // wait for the handler to be ready before sending the next
                // message
                vtask_hardware_delay(10);
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
