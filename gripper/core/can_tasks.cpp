#include <span>

#include "eeprom/core/message_handler.hpp"
#include "gripper/core/can_task.hpp"

using namespace can::dispatch;

static auto& main_queues = gripper_tasks::get_main_queues();
static auto& z_queues = gripper_tasks::z_tasks::get_queues();
static auto& g_queues = gripper_tasks::g_tasks::get_queues();

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    can::message_writer_task::TaskMessage>{};

/** The parsed message handler */
static auto can_motor_handler =
    can::message_handlers::motor::MotorHandler{z_queues};
static auto can_move_group_handler =
    can::message_handlers::move_group::MoveGroupHandler{z_queues};
static auto can_motion_handler =
    can::message_handlers::motion::MotionHandler{z_queues};
static auto can_brushed_motor_handler =
    can::message_handlers::motor::BrushedMotorHandler{g_queues};
static auto can_brushed_motion_handler =
    can::message_handlers::motion::BrushedMotionHandler{g_queues};
static auto can_brushed_move_group_handler =
    can::message_handlers::move_group::BrushedMoveGroupHandler{g_queues};
static auto gripper_info_handler =
    gripper_info::GripperInfoMessageHandler{main_queues, main_queues};
static auto sensor_handler = sensors::handlers::SensorHandler{main_queues};

/** Handler of system messages. */
static auto system_message_handler =
    can::message_handlers::system::SystemMessageHandler{
        main_queues,
        version_get()->version,
        version_get()->flags,
        std::span(std::cbegin(version_get()->sha),
                  std::cend(version_get()->sha)),
        revision_get()->primary,
        revision_get()->secondary,
        revision_get()->tertiary
        };
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

static auto motor_dispatch_target =
    can_task::MotorDispatchTarget{can_motor_handler};

static auto motion_group_dispatch_target =
    can_task::MoveGroupDispatchTarget{can_move_group_handler};

static auto motion_dispatch_target =
    can_task::MotionControllerDispatchTarget{can_motion_handler};

static auto brushed_motor_dispatch_target =
    can_task::BrushedMotorDispatchTarget{can_brushed_motor_handler};

static auto brushed_motion_dispatch_target =
    can_task::BrushedMotionDispatchTarget{can_brushed_motion_handler};

static auto brushed_motion_group_dispatch_target =
    can_task::BrushedMoveGroupDispatchTarget{can_brushed_move_group_handler};

static auto gripper_info_dispatch_target =
    can_task::GripperInfoDispatchTarget{gripper_info_handler};

static auto sensor_dispatch_target =
    can_task::SensorDispatchTarget{sensor_handler};

struct CheckForNodeId {
    can::ids::NodeId node_id;
    auto operator()(uint32_t arbitration_id) const {
        auto arb = can::arbitration_id::ArbitrationId(arbitration_id);
        auto _node_id = arb.node_id();
        return ((_node_id == node_id) ||
                (_node_id == can::ids::NodeId::broadcast) ||
                (_node_id == can::ids::NodeId::gripper));
    }
};

CheckForNodeId check_for_z{.node_id = can::ids::NodeId::gripper_z};

CheckForNodeId check_for_g{.node_id = can::ids::NodeId::gripper_g};

static auto dispatcher_z = can::dispatch::Dispatcher(
    check_for_z, motor_dispatch_target, motion_group_dispatch_target,
    motion_dispatch_target);

static auto dispatcher_g = can::dispatch::Dispatcher(
    check_for_g, brushed_motion_dispatch_target, brushed_motor_dispatch_target,
    brushed_motion_group_dispatch_target);

/** Dispatcher to the various handlers */
static auto main_dispatcher = can::dispatch::Dispatcher(
    [](uint32_t arbitration_id) -> bool {
        auto arb = can::arbitration_id::ArbitrationId(arbitration_id);
        auto node_id = arb.node_id();
        return ((node_id == can::ids::NodeId::broadcast) ||
                (node_id == can::ids::NodeId::gripper) ||
                (node_id == can::ids::NodeId::gripper_z) ||
                (node_id == can::ids::NodeId::gripper_g));
    },
    dispatcher_z, dispatcher_g, system_dispatch_target,
    gripper_info_dispatch_target, eeprom_dispatch_target,
    sensor_dispatch_target);

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
 * TODO (2021-12-15, AL): Most of what happens in this task should be moved out
 *  when we move to separate motor tasks.
 */
[[noreturn]] void can_task::CanMessageReaderTask::operator()(
    can::bus::CanBus* can_bus) {
    can_bus->set_incoming_message_callback(nullptr, callback);

    auto filter = can::arbitration_id::ArbitrationId();

    // Accept broadcast
    filter.node_id(can::ids::NodeId::broadcast);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo0, filter,
                        can::arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept any gripper
    filter.node_id(can::ids::NodeId::gripper);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can::arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept gripper z
    filter.node_id(can::ids::NodeId::gripper_z);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can::arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept gripper g
    filter.node_id(can::ids::NodeId::gripper_g);
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
