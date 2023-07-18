#pragma once

#include <vector>

#include "common/core/bit_utils.hpp"
#include "eeprom/core/data_rev.hpp"
#include "eeprom/core/dev_data.hpp"
#include "eeprom/core/task.hpp"

namespace eeprom {
namespace data_rev_task {

struct DataTableUpdateMessage {
    uint16_t data_rev;
    // list of new data table key/length pairs to add to the table
    std::vector<std::pair<types::address, types::data_length>> data_table;
};

using TaskMessage = std::variant<std::monostate, DataTableUpdateMessage>;

template <task::TaskClient EEPromClient>
class UpdateDataRevHandler : accessor::ReadListener {
  public:
    UpdateDataRevHandler(
        EEPromClient& eeprom_client,
        dev_data::DevDataTailAccessor<EEPromClient>& tail_accessor)
        : table_creator{eeprom_client, *this, accessor_backing, tail_accessor},
          data_rev_accessor{eeprom_client, *this, data_rev_backing} {
        data_rev_accessor.start_read(0);
    }

    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->visit(m); }, message);
    }

    auto ready() -> bool {
        return ready_for_new_message && table_creator.table_ready();
    }

  private:
    void visit(std::monostate&) {}

    void visit(const DataTableUpdateMessage& m) {
        // we really want to do this sequentially or the table can be malformed
        if (m.data_rev == current_data_rev + 1) {
            for (const auto& i : m.data_table) {
                // add the new data table entry
                table_creator.create_data_part(i.first, i.second);
                // wait for the table update to finish
                while (!table_creator.table_ready()) {
                    vTaskDelay(10);
                }
            }
            std::ignore = bit_utils::int_to_bytes(
                m.data_rev, data_rev_backing.begin(), data_rev_backing.end());
            data_rev_accessor.write(data_rev_backing, 0);
            current_data_rev = m.data_rev;
        }
    }

    void read_complete(uint32_t) final {
        // test if data is set to 0xFFFF, some chips default to 0x0000 but this
        // is fine since if it's set to that then it's already where we want it
        // to be as a default
        auto delivery_state =
            std::vector<uint8_t>(addresses::data_revision_length, 0xFF);
        if (std::equal(delivery_state.begin(), delivery_state.end(),
                       data_rev_backing.begin())) {
            data_rev_backing.fill(0x00);
            data_rev_accessor.write(data_rev_backing, 0);
        }

        std::ignore = bit_utils::bytes_to_int(
            data_rev_backing.begin(), data_rev_backing.end(), current_data_rev);
        ready_for_new_message = true;
    }
    uint16_t current_data_rev = 0;
    bool ready_for_new_message = false;
    dev_data::DataBufferType<8> accessor_backing =
        dev_data::DataBufferType<8>{};
    dev_data::DevDataAccessor<EEPromClient> table_creator;
    data_revision::DataRevisionType data_rev_backing =
        data_revision::DataRevisionType{};
    data_revision::DataRevAccessor<EEPromClient> data_rev_accessor;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class UpdateDataRevTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    UpdateDataRevTask(QueueType& queue) : queue{queue} {}
    UpdateDataRevTask(const UpdateDataRevTask& c) = delete;
    UpdateDataRevTask(const UpdateDataRevTask&& c) = delete;
    auto operator=(const UpdateDataRevTask& c) = delete;
    auto operator=(const UpdateDataRevTask&& c) = delete;
    ~UpdateDataRevTask() = default;

    /**
     * Task entry point.
     */
    template <task::TaskClient EEPromClient>
    void operator()(
        EEPromClient* eeprom_client,
        dev_data::DevDataTailAccessor<EEPromClient>* tail_accessor,
        const std::vector<eeprom::data_rev_task::DataTableUpdateMessage>*
            table_updater) {
        auto handler = UpdateDataRevHandler(*eeprom_client, *tail_accessor);
        for (const auto& i : *table_updater) {
            while (!handler.ready()) {
                vTaskDelay(10);
            }
            handler.handle_message(i);
        }
        tail_accessor->finish_data_rev();
        vTaskDelete(nullptr);
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
};

}  // namespace data_rev_task
}  // namespace eeprom