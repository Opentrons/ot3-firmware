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
#include "sensors/core/message_handlers/sensors.hpp"

namespace dispatch_builder {

using TMC2130MotorDispatchTarget = can_dispatch::DispatchParseTarget<
    motor_message_handler::MotorHandler<
        linear_motor_tasks::tmc2130_driver::QueueClient>,
    can_messages::ReadMotorDriverRegister, can_messages::SetupRequest,
    can_messages::WriteMotorDriverRegister,
    can_messages::WriteMotorCurrentRequest>;

using TMC2160MotorDispatchTarget = can_dispatch::DispatchParseTarget<
    motor_message_handler::MotorHandler<
        linear_motor_tasks::tmc2160_driver::QueueClient>,
    can_messages::ReadMotorDriverRegister, can_messages::SetupRequest,
    can_messages::WriteMotorDriverRegister,
    can_messages::WriteMotorCurrentRequest>;

using MoveGroupDispatchTarget = can_dispatch::DispatchParseTarget<
    move_group_handler::MoveGroupHandler<linear_motor_tasks::QueueClient>,
    can_messages::AddLinearMoveRequest, can_messages::ClearAllMoveGroupsRequest,
    can_messages::ExecuteMoveGroupRequest, can_messages::GetMoveGroupRequest,
    can_messages::HomeRequest>;

using MotionControllerDispatchTarget = can_dispatch::DispatchParseTarget<
    motion_message_handler::MotionHandler<linear_motor_tasks::QueueClient>,
    can_messages::DisableMotorRequest, can_messages::EnableMotorRequest,
    can_messages::GetMotionConstraintsRequest,
    can_messages::SetMotionConstraints, can_messages::StopRequest,
    can_messages::ReadLimitSwitchRequest>;

using SystemDispatchTarget = can_dispatch::DispatchParseTarget<
    system_handler::SystemMessageHandler<central_tasks::QueueClient>,
    can_messages::DeviceInfoRequest, can_messages::InitiateFirmwareUpdate,
    can_messages::FirmwareUpdateStatusRequest, can_messages::TaskInfoRequest>;

using SensorDispatchTarget = can_dispatch::DispatchParseTarget<
    sensors::handlers::SensorHandler<sensor_tasks::QueueClient>,
    can_messages::ReadFromSensorRequest, can_messages::WriteToSensorRequest,
    can_messages::BaselineSensorRequest,
    can_messages::SetSensorThresholdRequest,
    can_messages::BindSensorOutputRequest,
    can_messages::PeripheralStatusRequest>;

using PipetteInfoDispatchTarget = can_dispatch::DispatchParseTarget<
    pipette_info::PipetteInfoMessageHandler<central_tasks::QueueClient,
                                            sensor_tasks::QueueClient>,
    can_messages::PipetteInfoRequest>;

using EEPromDispatchTarget = can_dispatch::DispatchParseTarget<
    eeprom::message_handler::EEPromHandler<sensor_tasks::QueueClient,
                                           sensor_tasks::QueueClient>,
    can_messages::WriteToEEPromRequest, can_messages::ReadFromEEPromRequest>;

// TODO update with the proper message handler for pipette pick up tip
// motion handler.

}  // namespace dispatch_builder
