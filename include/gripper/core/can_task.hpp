#pragma once

#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/system.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "gripper/core/gripper_info.hpp"
#include "gripper/core/tasks.hpp"
#include "sensors/core/message_handlers/sensors.hpp"

namespace can_task {

using namespace gripper_tasks;

using MotorDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motor::MotorHandler<z_tasks::QueueClient>,
    can::messages::ReadMotorDriverRegister,
    can::messages::WriteMotorDriverRegister,
    can::messages::WriteMotorCurrentRequest,
    can::messages::ReadMotorDriverErrorStatus>;
using MoveGroupDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::move_group::MoveGroupHandler<z_tasks::QueueClient>,
    can::messages::AddLinearMoveRequest,
    can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::HomeRequest, can::messages::StopRequest>;
using MotionControllerDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motion::MotionHandler<z_tasks::QueueClient>,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::ReadLimitSwitchRequest,
    can::messages::MotorPositionRequest,
    can::messages::UpdateMotorPositionEstimationRequest,
    can::messages::GetMotorUsageRequest>;
using SystemDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::system::SystemMessageHandler<
        gripper_tasks::QueueClient>,
    can::messages::DeviceInfoRequest, can::messages::InitiateFirmwareUpdate,
    can::messages::FirmwareUpdateStatusRequest, can::messages::TaskInfoRequest>;
using BrushedMotorDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motor::BrushedMotorHandler<g_tasks::QueueClient>,
    can::messages::SetBrushedMotorVrefRequest,
    can::messages::SetBrushedMotorPwmRequest,
    can::messages::BrushedMotorConfRequest>;
using BrushedMotionDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motion::BrushedMotionHandler<g_tasks::QueueClient>,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::ReadLimitSwitchRequest, can::messages::MotorPositionRequest,
    can::messages::SetGripperErrorToleranceRequest,
    can::messages::GetMotorUsageRequest, can::messages::GripperJawStateRequest>;
using BrushedMoveGroupDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::move_group::BrushedMoveGroupHandler<
        g_tasks::QueueClient>,
    can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::GripperGripRequest, can::messages::GripperHomeRequest,
    can::messages::AddBrushedLinearMoveRequest, can::messages::StopRequest>;
using GripperInfoDispatchTarget = can::dispatch::DispatchParseTarget<
    gripper_info::GripperInfoMessageHandler<gripper_tasks::QueueClient,
                                            gripper_tasks::QueueClient>,
    can::messages::InstrumentInfoRequest, can::messages::SetSerialNumber>;
using SensorDispatchTarget = can::dispatch::DispatchParseTarget<
    sensors::handlers::SensorHandler<gripper_tasks::QueueClient>,
    can::messages::TipStatusQueryRequest, can::messages::ReadFromSensorRequest,
    can::messages::WriteToSensorRequest, can::messages::BaselineSensorRequest,
    can::messages::SetSensorThresholdRequest,
    can::messages::BindSensorOutputRequest,
    can::messages::PeripheralStatusRequest>;

auto constexpr reader_message_buffer_size = 1024;

struct CanMessageReaderTask {
    [[noreturn]] void operator()(can::bus::CanBus* can_bus);
};

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
