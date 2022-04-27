#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/system.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "gripper/core/gripper_info.hpp"
#include "gripper/core/tasks.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"

namespace can_bus {
class CanBus;
}

namespace can_task {

using MotorDispatchTarget = can_dispatch::DispatchParseTarget<
    motor_message_handler::MotorHandler<gripper_tasks::QueueClient>,
    can_messages::ReadMotorDriverRegister, can_messages::SetupRequest,
    can_messages::WriteMotorDriverRegister,
    can_messages::WriteMotorCurrentRequest>;
using MoveGroupDispatchTarget = can_dispatch::DispatchParseTarget<
    move_group_handler::MoveGroupHandler<gripper_tasks::QueueClient>,
    can_messages::AddLinearMoveRequest, can_messages::ClearAllMoveGroupsRequest,
    can_messages::ExecuteMoveGroupRequest, can_messages::GetMoveGroupRequest,
    can_messages::HomeRequest>;
using MotionControllerDispatchTarget = can_dispatch::DispatchParseTarget<
    motion_message_handler::MotionHandler<gripper_tasks::QueueClient>,
    can_messages::DisableMotorRequest, can_messages::EnableMotorRequest,
    can_messages::GetMotionConstraintsRequest,
    can_messages::SetMotionConstraints, can_messages::StopRequest,
    can_messages::ReadLimitSwitchRequest, can_messages::EncoderPositionRequest>;
using SystemDispatchTarget = can_dispatch::DispatchParseTarget<
    system_handler::SystemMessageHandler<gripper_tasks::QueueClient>,
    can_messages::DeviceInfoRequest, can_messages::InitiateFirmwareUpdate,
    can_messages::FirmwareUpdateStatusRequest, can_messages::TaskInfoRequest>;
using BrushedMotorDispatchTarget = can_dispatch::DispatchParseTarget<
    motor_message_handler::BrushedMotorHandler<gripper_tasks::QueueClient>,
    can_messages::SetupRequest, can_messages::SetBrushedMotorVrefRequest,
    can_messages::SetBrushedMotorPwmRequest>;
using BrushedMotionDispatchTarget = can_dispatch::DispatchParseTarget<
    motion_message_handler::BrushedMotionHandler<gripper_tasks::QueueClient>,
    can_messages::DisableMotorRequest, can_messages::EnableMotorRequest,
    can_messages::GripperGripRequest, can_messages::GripperHomeRequest>;
using GripperInfoDispatchTarget = can_dispatch::DispatchParseTarget<
    gripper_info::GripperInfoMessageHandler<gripper_tasks::QueueClient>,
    can_messages::GripperInfoRequest>;

using GripperDispatcherType =
    can_dispatch::Dispatcher<MotorDispatchTarget, MoveGroupDispatchTarget,
                             MotionControllerDispatchTarget,
                             SystemDispatchTarget, BrushedMotorDispatchTarget,
                             BrushedMotionDispatchTarget,
                             GripperInfoDispatchTarget>;

auto constexpr reader_message_buffer_size = 1024;
using CanMessageReaderTask =
    freertos_can_dispatch::FreeRTOSCanReader<reader_message_buffer_size,
                                             GripperDispatcherType>;

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