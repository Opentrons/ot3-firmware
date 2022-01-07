#include "head/core/can_task.hpp"

#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_handlers/device_info.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/presence_sensing.hpp"
#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "head/core/tasks.hpp"

static auto& right_queues = head_tasks::get_right_queues();
static auto& left_queues = head_tasks::get_left_queues();

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    message_writer_task::TaskMessage>{};

using MotorDispatchTarget = can_dispatch::DispatchParseTarget<
    motor_message_handler::MotorHandler<head_tasks::MotorQueueClient>,
    can_messages::ReadMotorDriverRegister, can_messages::SetupRequest,
    can_messages::WriteMotorDriverRegister>;
using MoveGroupDispatchTarget = can_dispatch::DispatchParseTarget<
    move_group_handler::MoveGroupHandler<head_tasks::MotorQueueClient>,
    can_messages::AddLinearMoveRequest, can_messages::ClearAllMoveGroupsRequest,
    can_messages::ExecuteMoveGroupRequest, can_messages::GetMoveGroupRequest>;
using MotionControllerDispatchTarget = can_dispatch::DispatchParseTarget<
    motion_message_handler::MotionHandler<head_tasks::MotorQueueClient>,
    can_messages::DisableMotorRequest, can_messages::EnableMotorRequest,
    can_messages::GetMotionConstraintsRequest,
    can_messages::SetMotionConstraints, can_messages::StopRequest>;
using DeviceInfoDispatchTarget = can_dispatch::DispatchParseTarget<
    device_info_handler::DeviceInfoHandler<head_tasks::MotorQueueClient>,
    can_messages::DeviceInfoRequest>;

/** The parsed message handler */
static auto can_motor_handler_right =
    motor_message_handler::MotorHandler{right_queues};
static auto can_motor_handler_left =
    motor_message_handler::MotorHandler{left_queues};

static auto can_motion_handler_right =
    motion_message_handler::MotionHandler{right_queues};
static auto can_motion_handler_left =
    motion_message_handler::MotionHandler{left_queues};

static auto can_move_group_handler_right =
    move_group_handler::MoveGroupHandler(right_queues);
static auto can_move_group_handler_left =
    move_group_handler::MoveGroupHandler(left_queues);

/** Handler of device info requests. */
static auto device_info_handler_right =
    device_info_handler::DeviceInfoHandler(right_queues, 0);
static auto device_info_dispatch_target_right =
    DeviceInfoDispatchTarget{device_info_handler_right};

static auto device_info_handler_left =
    device_info_handler::DeviceInfoHandler(left_queues, 0);
static auto device_info_dispatch_target_left =
    DeviceInfoDispatchTarget{device_info_handler_left};

static auto motor_dispatch_target_right =
    MotorDispatchTarget{can_motor_handler_right};
static auto motor_dispatch_target_left =
    MotorDispatchTarget{can_motor_handler_left};

static auto motion_dispatch_target_right =
    MotionControllerDispatchTarget{can_motion_handler_right};
static auto motion_dispatch_target_left =
    MotionControllerDispatchTarget{can_motion_handler_left};

static auto move_group_dispatch_target_right =
    MoveGroupDispatchTarget{can_move_group_handler_right};
static auto move_group_dispatch_target_left =
    MoveGroupDispatchTarget{can_move_group_handler_left};

/**
 * messages to head act like messages to both, head-right and head-left
 */

struct CheckForNodeId {
    can_ids::NodeId node_id;
    auto operator()(uint32_t arbitration_id) const {
        auto arb = can_arbitration_id::ArbitrationId(arbitration_id);
        auto _node_id = arb.node_id();
        return ((_node_id == node_id) ||
                (_node_id == can_ids::NodeId::broadcast) ||
                (_node_id == can_ids::NodeId::head));
    }
};

CheckForNodeId check_for_node_id_left{.node_id = can_ids::NodeId::head_l};

CheckForNodeId check_for_node_id_right{.node_id = can_ids::NodeId::head_r};

/** Dispatcher to the various right motor handlers */
static auto dispatcher_right_motor = can_dispatch::Dispatcher(
    check_for_node_id_right, motor_dispatch_target_right,
    motion_dispatch_target_right, move_group_dispatch_target_right,
    device_info_dispatch_target_right);

/** Dispatcher to the various left motor handlers */
static auto dispatcher_left_motor = can_dispatch::Dispatcher(
    check_for_node_id_left, motor_dispatch_target_left,
    motion_dispatch_target_left, move_group_dispatch_target_left,
    device_info_dispatch_target_left);

static auto main_dispatcher =
    can_dispatch::Dispatcher([](auto _) -> bool { return true; },
                             dispatcher_right_motor, dispatcher_left_motor);

/**
 * The type of the message buffer populated by HAL ISR.
 */
static auto read_can_message_buffer =
    freertos_message_buffer::FreeRTOSMessageBuffer<1024>{};
static auto read_can_message_buffer_writer =
    can_message_buffer::CanMessageBufferWriter(read_can_message_buffer);

/**
 * New CAN message callback.
 *
 * @param identifier Arbitration id
 * @param data Message data
 * @param length Message data length
 */
void callback(void* cb_data, uint32_t identifier, uint8_t* data,
              uint8_t length) {
    read_can_message_buffer_writer.send_from_isr(identifier, data,
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

    // Accept any head
    filter.node_id(can_ids::NodeId::head);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can_arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept head right
    filter.node_id(can_ids::NodeId::head_r);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can_arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept head left
    filter.node_id(can_ids::NodeId::head_l);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can_arbitration_id::ArbitrationId::node_id_bit_mask);

    // Reject everything else.
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::reject, 0, 0);

    auto poller = freertos_can_dispatch::FreeRTOSCanBufferPoller(
        read_can_message_buffer, main_dispatcher);
    poller();
}

auto static reader_task = can_task::CanMessageReaderTask{};
auto static writer_task = can_task::CanMessageWriterTask{can_sender_queue};

auto static reader_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageReaderTask,
                                can_bus::CanBus>{reader_task};
auto static writer_task_control =
    freertos_task::FreeRTOSTask<512, can_task::CanMessageWriterTask,
                                can_bus::CanBus>{writer_task};

auto can_task::start_reader(can_bus::CanBus& canbus)
    -> can_task::CanMessageReaderTask& {
    reader_task_control.start(5, "can reader task", &canbus);
    return reader_task;
}

auto can_task::start_writer(can_bus::CanBus& canbus)
    -> can_task::CanMessageWriterTask& {
    writer_task_control.start(5, "can writer task", &canbus);
    return writer_task;
}