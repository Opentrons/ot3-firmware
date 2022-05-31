#pragma once

#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/system.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "gripper/core/gripper_info.hpp"
#include "gripper/core/tasks.hpp"

namespace can_bus {
class CanBus;
}

namespace can_task {

using namespace gripper_tasks;

using MotorDispatchTarget = can_dispatch::DispatchParseTarget<
    motor_message_handler::MotorHandler<z_tasks::QueueClient>,
    can_messages::ReadMotorDriverRegister, can_messages::SetupRequest,
    can_messages::WriteMotorDriverRegister,
    can_messages::WriteMotorCurrentRequest>;
using MoveGroupDispatchTarget = can_dispatch::DispatchParseTarget<
    move_group_handler::MoveGroupHandler<z_tasks::QueueClient>,
    can_messages::AddLinearMoveRequest, can_messages::ClearAllMoveGroupsRequest,
    can_messages::ExecuteMoveGroupRequest, can_messages::GetMoveGroupRequest,
    can_messages::HomeRequest>;
using MotionControllerDispatchTarget = can_dispatch::DispatchParseTarget<
    motion_message_handler::MotionHandler<z_tasks::QueueClient>,
    can_messages::DisableMotorRequest, can_messages::EnableMotorRequest,
    can_messages::GetMotionConstraintsRequest,
    can_messages::SetMotionConstraints, can_messages::StopRequest,
    can_messages::ReadLimitSwitchRequest>;
using SystemDispatchTarget = can_dispatch::DispatchParseTarget<
    system_handler::SystemMessageHandler<gripper_tasks::QueueClient>,
    can_messages::DeviceInfoRequest, can_messages::InitiateFirmwareUpdate,
    can_messages::FirmwareUpdateStatusRequest, can_messages::TaskInfoRequest>;
using BrushedMotorDispatchTarget = can_dispatch::DispatchParseTarget<
    motor_message_handler::BrushedMotorHandler<g_tasks::QueueClient>,
    can_messages::SetupRequest, can_messages::SetBrushedMotorVrefRequest,
    can_messages::SetBrushedMotorPwmRequest>;
using BrushedMotionDispatchTarget = can_dispatch::DispatchParseTarget<
    motion_message_handler::BrushedMotionHandler<g_tasks::QueueClient>,
    can_messages::DisableMotorRequest, can_messages::EnableMotorRequest,
    can_messages::StopRequest, can_messages::ReadLimitSwitchRequest>;
using BrushedMoveGroupDispatchTarget = can_dispatch::DispatchParseTarget<
    move_group_handler::BrushedMoveGroupHandler<g_tasks::QueueClient>,
    can_messages::ClearAllMoveGroupsRequest,
    can_messages::ExecuteMoveGroupRequest, can_messages::GetMoveGroupRequest,
    can_messages::GripperGripRequest, can_messages::GripperHomeRequest>;
using GripperInfoDispatchTarget = can_dispatch::DispatchParseTarget<
    gripper_info::GripperInfoMessageHandler<gripper_tasks::QueueClient,
                                            gripper_tasks::QueueClient>,
    can_messages::GripperInfoRequest, can_messages::InstrumentInfoRequest,
    can_messages::SetSerialNumber>;

auto constexpr reader_message_buffer_size = 1024;

struct CanMessageReaderTask {
    [[noreturn]] void operator()(can_bus::CanBus* can_bus);
};

/**
 * Create the can message reader task.
 *
 * @param canbus reference to the can bus
 * @return The task.
 */
auto start_reader(can_bus::CanBus& canbus) -> CanMessageReaderTask&;

using CanMessageWriterTask = message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>;

/**
 * Create the can message writer task.
 *
 * @param canbus reference to the can bus
 * @return The task.
 */
auto start_writer(can_bus::CanBus& canbus) -> CanMessageWriterTask&;

}  // namespace can_task