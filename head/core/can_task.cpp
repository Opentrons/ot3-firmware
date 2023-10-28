#include "head/core/can_task.hpp"

#include <span>

#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/presence_sensing.hpp"
#include "can/core/message_handlers/system.hpp"
#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/version.h"
#include "eeprom/core/message_handler.hpp"
#include "head/core/queues.hpp"

static auto& right_queues = head_tasks::get_right_queues();
static auto& left_queues = head_tasks::get_left_queues();
static auto& common_queues = head_tasks::get_queue_client();

auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    can::message_writer_task::TaskMessage>{};

using MotorDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motor::MotorHandler<head_tasks::MotorQueueClient>,
    can::messages::ReadMotorDriverRegister,
    can::messages::WriteMotorDriverRegister,
    can::messages::WriteMotorCurrentRequest,
    can::messages::ReadMotorDriverErrorStatus>;
using MoveGroupDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::move_group::MoveGroupHandler<
        head_tasks::MotorQueueClient>,
    can::messages::AddLinearMoveRequest,
    can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::HomeRequest, can::messages::StopRequest>;
using MotionControllerDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motion::MotionHandler<head_tasks::MotorQueueClient>,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::ReadLimitSwitchRequest,
    can::messages::MotorPositionRequest,
    can::messages::UpdateMotorPositionEstimationRequest,
    can::messages::GetMotorUsageRequest,
    can::messages::MotorDriverErrorEncountered,
    can::messages::ResetMotorDriverErrorHandling>;
using SystemDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::system::SystemMessageHandler<
        head_tasks::HeadQueueClient>,
    can::messages::DeviceInfoRequest, can::messages::InitiateFirmwareUpdate,
    can::messages::FirmwareUpdateStatusRequest, can::messages::TaskInfoRequest>;
using PresenceSensingDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::presence_sensing::PresenceSensingHandler<
        head_tasks::HeadQueueClient>,
    can::messages::AttachedToolsRequest>;

/** The parsed message handler */
static auto presence_sensing_handler =
    can::message_handlers::presence_sensing::PresenceSensingHandler{
        common_queues};
static auto can_motor_handler_right =
    can::message_handlers::motor::MotorHandler{right_queues};
static auto can_motor_handler_left =
    can::message_handlers::motor::MotorHandler{left_queues};

static auto can_motion_handler_right =
    can::message_handlers::motion::MotionHandler{right_queues};
static auto can_motion_handler_left =
    can::message_handlers::motion::MotionHandler{left_queues};

static auto can_move_group_handler_right =
    can::message_handlers::move_group::MoveGroupHandler(right_queues);
static auto can_move_group_handler_left =
    can::message_handlers::move_group::MoveGroupHandler(left_queues);

/** Handler of system messages. */
static auto system_message_handler =
    can::message_handlers::system::SystemMessageHandler(
        common_queues, version_get()->version, version_get()->flags,
        std::span(std::cbegin(version_get()->sha),
                  std::cend(version_get()->sha)),
        revision_get()->primary, revision_get()->secondary);
static auto system_dispatch_target =
    SystemDispatchTarget{system_message_handler};

/** Handler for eeprom messages.*/
static auto eeprom_message_handler =
    eeprom::message_handler::EEPromHandler{common_queues, common_queues};
static auto eeprom_dispatch_target =
    can::dispatch::DispatchParseTarget<decltype(eeprom_message_handler),
                                       can::messages::WriteToEEPromRequest,
                                       can::messages::ReadFromEEPromRequest>{
        eeprom_message_handler};

static auto presence_sensing_dispatch_target =
    PresenceSensingDispatchTarget{presence_sensing_handler};

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
    can::ids::NodeId node_id;
    auto operator()(uint32_t arbitration_id) const {
        auto arb = can::arbitration_id::ArbitrationId(arbitration_id);
        auto _node_id = arb.node_id();
        return ((_node_id == node_id) ||
                (_node_id == can::ids::NodeId::broadcast) ||
                (_node_id == can::ids::NodeId::head));
    }
};

CheckForNodeId check_for_node_id_left{.node_id = can::ids::NodeId::head_l};

CheckForNodeId check_for_node_id_right{.node_id = can::ids::NodeId::head_r};

/** Dispatcher to the various right motor handlers */
static auto dispatcher_right_motor = can::dispatch::Dispatcher(
    check_for_node_id_right, motor_dispatch_target_right,
    motion_dispatch_target_right, move_group_dispatch_target_right);

/** Dispatcher to the various left motor handlers */
static auto dispatcher_left_motor = can::dispatch::Dispatcher(
    check_for_node_id_left, motor_dispatch_target_left,
    motion_dispatch_target_left, move_group_dispatch_target_left);

static auto main_dispatcher = can::dispatch::Dispatcher(
    [](uint32_t arbitration_id) -> bool {
        auto arb = can::arbitration_id::ArbitrationId(arbitration_id);
        auto node_id = arb.node_id();
        return ((node_id == can::ids::NodeId::broadcast) ||
                (node_id == can::ids::NodeId::head) ||
                (node_id == can::ids::NodeId::head_l) ||
                (node_id == can::ids::NodeId::head_r));
    },
    dispatcher_right_motor, dispatcher_left_motor,
    presence_sensing_dispatch_target, system_dispatch_target,
    eeprom_dispatch_target);

/**
 * The type of the message buffer populated by HAL ISR.
 */
static auto read_can_message_buffer =
    freertos_message_buffer::FreeRTOSMessageBuffer<1024>{};
static auto read_can_message_buffer_writer =
    can::message_buffer::CanMessageBufferWriter(read_can_message_buffer);

/**
 * New CAN message callback.
 *
 * @param identifier Arbitration id
 * @param data Message data
 * @param length Message data length
 */
void callback(void*, uint32_t identifier, uint8_t* data, uint8_t length) {
    read_can_message_buffer_writer.send_from_isr(identifier, data,
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

    // Accept any head
    filter.node_id(can::ids::NodeId::head);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can::arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept head right
    filter.node_id(can::ids::NodeId::head_r);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can::arbitration_id::ArbitrationId::node_id_bit_mask);

    // Accept head left
    filter.node_id(can::ids::NodeId::head_l);
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::to_fifo1, filter,
                        can::arbitration_id::ArbitrationId::node_id_bit_mask);

    // Reject everything else.
    can_bus->add_filter(CanFilterType::mask, CanFilterConfig::reject, 0, 0);

    auto poller = can::freertos_dispatch::FreeRTOSCanBufferPoller(
        read_can_message_buffer, main_dispatcher);
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

auto can_task::start_reader(can::bus::CanBus& canbus)
    -> can_task::CanMessageReaderTask& {
    reader_task_control.start(5, "can reader task", &canbus);
    return reader_task;
}

auto can_task::start_writer(can::bus::CanBus& canbus)
    -> can_task::CanMessageWriterTask& {
    writer_task_control.start(5, "can writer task", &canbus);
    return writer_task;
}
