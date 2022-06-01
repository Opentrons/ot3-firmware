#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/system.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "gantry/core/tasks.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"

namespace can_bus {
class CanBus;
}

namespace can_task {

using MotorDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motor::MotorHandler<gantry_tasks::QueueClient>,
    can::messages::ReadMotorDriverRegister, can::messages::SetupRequest,
    can::messages::WriteMotorDriverRegister,
    can::messages::WriteMotorCurrentRequest>;
using MoveGroupDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::move_group::MoveGroupHandler<
        gantry_tasks::QueueClient>,
    can::messages::AddLinearMoveRequest,
    can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::HomeRequest>;
using MotionControllerDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motion::MotionHandler<gantry_tasks::QueueClient>,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::StopRequest,
    can::messages::ReadLimitSwitchRequest>;
using SystemDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::system::SystemMessageHandler<
        gantry_tasks::QueueClient>,
    can::messages::DeviceInfoRequest, can::messages::InitiateFirmwareUpdate,
    can::messages::FirmwareUpdateStatusRequest, can::messages::TaskInfoRequest>;

using GantryDispatcherType =
    can::dispatch::Dispatcher<MotorDispatchTarget, MoveGroupDispatchTarget,
                              MotionControllerDispatchTarget,
                              SystemDispatchTarget>;

auto constexpr reader_message_buffer_size = 1024;
using CanMessageReaderTask =
    can::freertos_dispatch::FreeRTOSCanReader<reader_message_buffer_size,
                                              GantryDispatcherType>;

/**
 * Create the can message reader task.
 *
 * @param canbus reference to the can bus
 * @return The task.
 */
auto start_reader(can::bus::CanBus& canbus) -> CanMessageReaderTask&;

using CanMessageWriterTask = can::message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>;

/**
 * Create the can message writer task.
 *
 * @param canbus reference to the can bus
 * @return The task.
 */
auto start_writer(can::bus::CanBus& canbus) -> CanMessageWriterTask&;

}  // namespace can_task