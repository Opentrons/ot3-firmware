#include <span>

#include "eeprom/core/message_handler.hpp"
#include "hepa-uv/core/can_task.hpp"
#include "hepa-uv/core/hepa_task.hpp"
#include "hepa-uv/core/hepauv_info.hpp"
#include "hepa-uv/core/message_handler.hpp"

using namespace can::dispatch;

static auto& main_queues = hepauv_tasks::get_main_queues();

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    can::message_writer_task::TaskMessage>{};

/** The parsed message handler */
static auto hepauv_info_handler =
    hepauv_info::HepaUVInfoMessageHandler{main_queues, main_queues};

static auto hepa_fan_handler = hepa::message_handler::HepaHandler{main_queues};
static auto uv_light_handler = uv::message_handler::UVHandler{main_queues};

/** Handler of system messages. */
static auto system_message_handler =
    can::message_handlers::system::SystemMessageHandler{
        main_queues,
        version_get()->version,
        version_get()->flags,
        std::span(std::cbegin(version_get()->sha),
                  std::cend(version_get()->sha)),
        revision_get()->primary,
        revision_get()->secondary};
static auto system_dispatch_target =
    can_task::SystemDispatchTarget{system_message_handler};

/** Handler for eeprom messages.*/
static auto eeprom_message_handler =
    eeprom::message_handler::EEPromHandler{main_queues, main_queues};
static auto eeprom_dispatch_target =
    can::dispatch::DispatchParseTarget<decltype(eeprom_message_handler),
                                       can::messages::WriteToEEPromRequest,
                                       can::messages::ReadFromEEPromRequest>{
        eeprom_message_handler};

static auto hepauv_info_dispatch_target =
    can_task::HepaUVInfoDispatchTarget{hepauv_info_handler};

static auto hepa_dispatch_target =
    can_task::HepaDispatchTarget{hepa_fan_handler};

static auto uv_dispatch_target = can_task::UVDispatchTarget{uv_light_handler};

struct CheckForNodeId {
    can::ids::NodeId node_id;
    auto operator()(uint32_t arbitration_id) const {
        auto arb = can::arbitration_id::ArbitrationId(arbitration_id);
        auto _node_id = arb.node_id();
        return ((_node_id == node_id) ||
                (_node_id == can::ids::NodeId::broadcast) ||
                (_node_id == can::ids::NodeId::hepa_uv));
    }
};

/** Dispatcher to the various handlers */
static auto main_dispatcher = can::dispatch::Dispatcher(
    [](uint32_t arbitration_id) -> bool {
        auto arb = can::arbitration_id::ArbitrationId(arbitration_id);
        auto node_id = arb.node_id();
        return ((node_id == can::ids::NodeId::broadcast) ||
                (node_id == can::ids::NodeId::hepa_uv));
    },
    system_dispatch_target, hepauv_info_dispatch_target, eeprom_dispatch_target,
    hepa_dispatch_target, uv_dispatch_target);

auto static reader_message_buffer =
    freertos_message_buffer::FreeRTOSMessageBuffer<1024>{};
static auto reader_message_buffer_writer =
    can::message_buffer::CanMessageBufferWriter(reader_message_buffer);

/**
 * New CAN message callback.
 *
 * @param identifier Arbitration id
 * @param data Message data
 * @param length Message data length
 */
void callback(void*, uint32_t identifier, uint8_t* data, uint8_t length) {
    reader_message_buffer_writer.send_from_isr(identifier, data,
                                               data + length);  // NOLINT
}

/**
 * Entry point for the reader task.
 */
[[noreturn]] void can_task::CanMessageReaderTask::operator()(
    can::bus::CanBus* can_bus) {
    can_bus->set_incoming_message_callback(nullptr, callback);

    auto filter = can::arbitration_id::ArbitrationId();

    // Accept broadcast
    filter.node_id(can::ids::NodeId::broadcast);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo0, filter,
                        can::arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept Hepa/UV
    filter.node_id(can::ids::NodeId::hepa_uv);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can::arbitration_id::ArbitrationId::node_id_bit_mask);

    // Reject everything else.
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::reject, 0, 0);

    auto poller = can::freertos_dispatch::FreeRTOSCanBufferPoller(
        reader_message_buffer, main_dispatcher);
    poller();
}

auto static reader_task = can_task::CanMessageReaderTask{};
auto static writer_task = can_task::CanMessageWriterTask{can_sender_queue};

auto static reader_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageReaderTask>{
        reader_task};
auto static writer_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageWriterTask>{
        writer_task};

/**
 * Start the can reader task
 * @param canbus The can bus reference
 * @return The task entry point.
 */
auto can_task::start_reader(can::bus::CanBus& canbus)
    -> can_task::CanMessageReaderTask& {
    LOG("Starting the CAN reader task");

    reader_task_control.start(5, "can reader task", &canbus);
    return reader_task;
}

/**
 * Start the can writer task
 * @param canbus The can bus reference
 * @return The task entry point
 */
auto can_task::start_writer(can::bus::CanBus& canbus)
    -> can_task::CanMessageWriterTask& {
    LOG("Starting the CAN writer task");

    writer_task_control.start(5, "can writer task", &canbus);
    return writer_task;
}
