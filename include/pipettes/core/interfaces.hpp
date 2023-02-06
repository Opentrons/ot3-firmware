#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"

#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "pipettes/firmware/pipette_motor_hardware.hpp"

namespace interfaces {

using DefinedMotorHardware = motor_hardware::MotorHardware;

using MoveQueue =
    freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>;
using GearMoveQueue =
    freertos_message_queue::FreeRTOSMessageQueue<motor_messages::GearMotorMove>;
using MotionControlType =
    motion_controller::MotionController<lms::LeadScrewConfig,
                                        DefinedMotorHardware>;
using PipetteMotionControlType =
    pipette_motion_controller::PipetteMotionController<
        lms::LeadScrewConfig, pipette_motor_hardware::MotorHardware>;
using UpdatePositionQueue = freertos_message_queue::FreeRTOSMessageQueue<
    can::messages::UpdateMotorPositionEstimationRequest>;

struct LowThroughputInterruptQueues {
    MoveQueue plunger_queue;
    UpdatePositionQueue plunger_update_queue;
};

struct HighThroughputInterruptQueues {
    MoveQueue plunger_queue;
    UpdatePositionQueue plunger_update_queue;
    GearMoveQueue right_motor_queue;
    UpdatePositionQueue right_update_queue;
    GearMoveQueue left_motor_queue;
    UpdatePositionQueue left_update_queue;
};

namespace gear_motor {

struct UnavailableGearHardware {};
struct UnavailableGearMotionControl {};
struct UnavailableGearInterrupts {};
struct UnavailableGearHardwareTasks {};

struct GearMotionControl {
    PipetteMotionControlType left;
    PipetteMotionControlType right;
};

struct GearMotorHardwareTasks {
    pipette_motor_hardware_task::PipetteMotorHardwareTask<
        pipette_motor_hardware::MotorHardware>
        left;
    pipette_motor_hardware_task::PipetteMotorHardwareTask<
        pipette_motor_hardware::MotorHardware>
        right;
};

}  // namespace gear_motor

}  // namespace interfaces
