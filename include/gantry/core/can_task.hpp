#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/device_info.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "gantry/core/tasks.hpp"
#include "motor-control/core/motor.hpp"

namespace can_bus {
class CanBus;
}

namespace can_task {

using MotorDispatchTarget = can_dispatch::DispatchParseTarget<
    motor_message_handler::MotorHandler<gantry_tasks::QueueClient>,
    can_messages::ReadMotorDriverRegister, can_messages::SetupRequest,
    can_messages::WriteMotorDriverRegister>;
using MoveGroupDispatchTarget = can_dispatch::DispatchParseTarget<
    move_group_handler::MoveGroupHandler<gantry_tasks::QueueClient>,
    can_messages::AddLinearMoveRequest, can_messages::ClearAllMoveGroupsRequest,
    can_messages::ExecuteMoveGroupRequest, can_messages::GetMoveGroupRequest>;
using MotionControllerDispatchTarget = can_dispatch::DispatchParseTarget<
    motion_message_handler::MotionHandler<gantry_tasks::QueueClient>,
    can_messages::DisableMotorRequest, can_messages::EnableMotorRequest,
    can_messages::GetMotionConstraintsRequest,
    can_messages::SetMotionConstraints, can_messages::StopRequest,
    can_messages::ReadLimitSwitchRequest>;
using SystemDispatchTarget = can_dispatch::DispatchParseTarget<
    system_handler::SystemMessageHandler<gantry_tasks::QueueClient>,
    can_messages::DeviceInfoRequest, can_messages::InitiateFirmwareUpdate>;

using GantryDispatcherType =
    can_dispatch::Dispatcher<MotorDispatchTarget, MoveGroupDispatchTarget,
                             MotionControllerDispatchTarget,
                             SystemDispatchTarget>;

auto constexpr reader_message_buffer_size = 1024;
using CanMessageReaderTask =
    freertos_can_dispatch::FreeRTOSCanReader<reader_message_buffer_size,
                                             GantryDispatcherType>;

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