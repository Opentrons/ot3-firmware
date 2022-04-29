#include <span>

#include "can/core/can_bus.hpp"
#include "can/core/can_writer_task.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/system.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.h"
#include "common/core/version.h"
#include "gripper/core/can_task.hpp"
#include "gripper/core/gripper_info.hpp"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"

using namespace can_dispatch;

static auto& main_queues = gripper_tasks::get_main_queues();
static auto& z_queues = gripper_tasks::z_tasks::get_queues();
static auto& g_queues = gripper_tasks::g_tasks::get_queues();

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    message_writer_task::TaskMessage>{};

/** The parsed message handler */
static auto can_motor_handler = motor_message_handler::MotorHandler{z_queues};
static auto can_move_group_handler =
    move_group_handler::MoveGroupHandler{z_queues};
static auto can_motion_handler =
    motion_message_handler::MotionHandler{z_queues};
static auto can_brushed_motor_handler =
    motor_message_handler::BrushedMotorHandler{g_queues};
static auto can_brushed_motion_handler =
    motion_message_handler::BrushedMotionHandler{g_queues};
static auto can_brushed_move_group_handler =
    move_group_handler::BrushedMoveGroupHandler{g_queues};
static auto gripper_info_handler =
    gripper_info::GripperInfoMessageHandler{main_queues};

/** Handler of system messages. */
static auto system_message_handler = system_handler::SystemMessageHandler{
    main_queues, version_get()->version, version_get()->flags,
    std::span(std::cbegin(version_get()->sha), std::cend(version_get()->sha))};
static auto system_dispatch_target =
    can_task::SystemDispatchTarget{system_message_handler};

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

struct CheckForNodeId {
    can_ids::NodeId node_id;
    auto operator()(uint32_t arbitration_id) const {
        auto arb = can_arbitration_id::ArbitrationId(arbitration_id);
        auto _node_id = arb.node_id();
        return ((_node_id == node_id) ||
                (_node_id == can_ids::NodeId::broadcast) ||
                (_node_id == can_ids::NodeId::gripper));
    }
};

CheckForNodeId check_for_z{.node_id = can_ids::NodeId::gripper_z};

CheckForNodeId check_for_g{.node_id = can_ids::NodeId::gripper_g};

static auto dispatcher_z = can_dispatch::Dispatcher(
    check_for_z, motor_dispatch_target, motion_group_dispatch_target,
    motion_dispatch_target);

static auto dispatcher_g = can_dispatch::Dispatcher(
    check_for_g, brushed_motion_dispatch_target, brushed_motor_dispatch_target,
    brushed_motion_group_dispatch_target);

/** Dispatcher to the various handlers */
static auto main_dispatcher = can_dispatch::Dispatcher(
    [](auto) -> bool { return true; }, dispatcher_z, dispatcher_g,
    system_dispatch_target, gripper_info_dispatch_target);

auto static reader_message_buffer =
    freertos_message_buffer::FreeRTOSMessageBuffer<1024>{};
static auto reader_message_buffer_writer =
    can_message_buffer::CanMessageBufferWriter(reader_message_buffer);

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
    can_bus::CanBus* can_bus) {
    can_bus->set_incoming_message_callback(nullptr, callback);

    auto filter = can_arbitration_id::ArbitrationId();

    // Accept broadcast
    filter.node_id(can_ids::NodeId::broadcast);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo0, filter,
                        can_arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept any gripper
    filter.node_id(can_ids::NodeId::gripper);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can_arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept gripper z
    filter.node_id(can_ids::NodeId::gripper_z);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can_arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept gripper g
    filter.node_id(can_ids::NodeId::gripper_g);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can_arbitration_id::ArbitrationId::node_id_bit_mask);

    // Reject everything else.
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::reject, 0, 0);

    auto poller = freertos_can_dispatch::FreeRTOSCanBufferPoller(
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
auto can_task::start_reader(can_bus::CanBus& canbus)
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
auto can_task::start_writer(can_bus::CanBus& canbus)
    -> can_task::CanMessageWriterTask& {
    LOG("Starting the CAN writer task");

    writer_task_control.start(5, "can writer task", &canbus);
    return writer_task;
}
