#pragma once

#include "can/core/dispatch.hpp"
#include "can/core/message_handlers/motion.hpp"
#include "can/core/message_handlers/motor.hpp"
#include "can/core/message_handlers/move_group.hpp"
#include "can/core/message_handlers/system.hpp"
#include "eeprom/core/message_handler.hpp"
#include "pipettes/core/central_tasks.hpp"
#include "pipettes/core/head_gear_tasks.hpp"
#include "pipettes/core/head_peripheral_tasks.hpp"
#include "pipettes/core/head_sensor_tasks.hpp"
#include "pipettes/core/pipette_info.hpp"
#include "pipettes/core/tasks/message_handlers/motion.hpp"
#include "pipettes/core/tasks/message_handlers/move_group.hpp"
#include "sensors/core/message_handlers/sensors.hpp"

namespace head_dispatch_builder {

using TMC2160MotorDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motor::MotorHandler<head_gear_tasks::QueueClient>,
    can::messages::ReadMotorDriverRegister,
    can::messages::WriteMotorDriverRegister,
    can::messages::WriteMotorCurrentRequest>;

using GearMotorDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::motor::MotorHandler<head_gear_tasks::QueueClient>,
    can::messages::ReadMotorDriverRegister,
    can::messages::WriteMotorDriverRegister,
    can::messages::WriteMotorCurrentRequest>;

using GearMoveGroupDispatchTarget = can::dispatch::DispatchParseTarget<
    gear_move_group_handler::GearMoveGroupHandler<head_gear_tasks::QueueClient>,
    can::messages::ClearAllMoveGroupsRequest,
    can::messages::ExecuteMoveGroupRequest, can::messages::GetMoveGroupRequest,
    can::messages::TipActionRequest>;

using GearMotionControllerDispatchTarget = can::dispatch::DispatchParseTarget<
    gear_motion_handler::GearMotorMotionHandler<head_gear_tasks::QueueClient>,
    can::messages::DisableMotorRequest, can::messages::EnableMotorRequest,
    can::messages::GetMotionConstraintsRequest,
    can::messages::SetMotionConstraints, can::messages::StopRequest,
    can::messages::ReadLimitSwitchRequest, can::messages::TipActionRequest>;

using SystemDispatchTarget = can::dispatch::DispatchParseTarget<
    can::message_handlers::system::SystemMessageHandler<
        central_tasks::QueueClient>,
    can::messages::DeviceInfoRequest, can::messages::InitiateFirmwareUpdate,
    can::messages::FirmwareUpdateStatusRequest, can::messages::TaskInfoRequest>;

using PipetteInfoDispatchTarget = can::dispatch::DispatchParseTarget<
    pipette_info::PipetteInfoMessageHandler<central_tasks::QueueClient,
                                            head_sensor_tasks::QueueClient>,
    can::messages::InstrumentInfoRequest, can::messages::SetSerialNumber>;

using EEPromDispatchTarget = can::dispatch::DispatchParseTarget<
    eeprom::message_handler::EEPromHandler<head_sensor_tasks::QueueClient,
                                           head_sensor_tasks::QueueClient>,
    can::messages::WriteToEEPromRequest, can::messages::ReadFromEEPromRequest>;

}  // namespace head_dispatch_builder
