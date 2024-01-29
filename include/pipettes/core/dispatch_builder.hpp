#pragma once

#include "can/core/dispatch.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/system.hpp"
#include "eeprom/core/message_handler.hpp"
#include "pipettes/core/central_tasks.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/peripheral_tasks.hpp"
#include "pipettes/core/pipette_info.hpp"
#include "pipettes/core/sensor_tasks.hpp"
#include "pipettes/core/tasks/message_handlers/motion.hpp"
#include "pipettes/core/tasks/message_handlers/move_group.hpp"
#include "sensors/core/message_handlers/sensors.hpp"

namespace dispatch_builder {

using TMC2130MotorDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motor::MotorHandler<
        linear_motor_tasks::tmc2130_driver::QueueClient>,
    can::messages::ReadMotorDriverRegister,
    can::messages::WriteMotorDriverRegister,
    can::messages::WriteMotorCurrentRequest>;

using TMC2160MotorDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motor::MotorHandler<
        linear_motor_tasks::tmc2160_driver::QueueClient>,
    can::messages::ReadMotorDriverRegister,
    can::messages::WriteMotorDriverRegister,
    can::messages::WriteMotorCurrentRequest>;

using GearMotorDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motor::GearMotorHandler<
        gear_motor_tasks::QueueClient>,
    can::messages::GearReadMotorDriverRegister,
    can::messages::GearWriteMotorDriverRegister,
    can::messages::GearWriteMotorCurrentRequest>;

#ifdef PIPETTE_TYPE_DEFINE
using MoveGroupDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::move_group::MoveGroupHandler<
        linear_motor_tasks::QueueClient>,
    can::messages::AddLinearMoveRequest,
    can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::HomeRequest, can::messages::StopRequest,
    can::messages::AddSensorMoveRequest>;
#else
using MoveGroupDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::move_group::MoveGroupHandler<
        linear_motor_tasks::QueueClient>,
    can::messages::AddLinearMoveRequest,
    can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::HomeRequest, can::messages::StopRequest>;
#endif

using GearMoveGroupDispatchTarget = can::dispatch::DispatchParseTarget<
    gear_move_group_handler::GearMoveGroupHandler<
        gear_motor_tasks::QueueClient>,
    can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::TipActionRequest, can::messages::StopRequest>;

using MotionControllerDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motion::MotionHandler<
        linear_motor_tasks::QueueClient>,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::ReadLimitSwitchRequest,
    can::messages::MotorPositionRequest,
    can::messages::UpdateMotorPositionEstimationRequest,
    can::messages::GetMotorUsageRequest>;

using GearMotionControllerDispatchTarget = can::dispatch::DispatchParseTarget<
    gear_motion_handler::GearMotorMotionHandler<gear_motor_tasks::QueueClient>,
    can::messages::GearDisableMotorRequest,
    can::messages::GearEnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::ReadLimitSwitchRequest,
    can::messages::GetMotorUsageRequest>;

using SystemDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::system::SystemMessageHandler<
        central_tasks::QueueClient>,
    can::messages::DeviceInfoRequest, can::messages::InitiateFirmwareUpdate,
    can::messages::FirmwareUpdateStatusRequest, can::messages::TaskInfoRequest>;

using SensorDispatchTarget = can::dispatch::DispatchParseTarget<
    sensors::handlers::SensorHandler<sensor_tasks::QueueClient>,
    can::messages::TipStatusQueryRequest, can::messages::ReadFromSensorRequest,
    can::messages::WriteToSensorRequest, can::messages::BaselineSensorRequest,
    can::messages::SetSensorThresholdRequest,
    can::messages::BindSensorOutputRequest,
    can::messages::PeripheralStatusRequest>;

using PipetteInfoDispatchTarget = can::dispatch::DispatchParseTarget<
    pipette_info::PipetteInfoMessageHandler<central_tasks::QueueClient,
                                            sensor_tasks::QueueClient>,
    can::messages::InstrumentInfoRequest, can::messages::SetSerialNumber>;

using EEPromDispatchTarget = can::dispatch::DispatchParseTarget<
    eeprom::message_handler::EEPromHandler<sensor_tasks::QueueClient,
                                           sensor_tasks::QueueClient>,
    can::messages::WriteToEEPromRequest, can::messages::ReadFromEEPromRequest>;

}  // namespace dispatch_builder
