#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"

namespace interfaces {

using MoveQueue =
    freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>;
using GearMoveQueue =
    freertos_message_queue::FreeRTOSMessageQueue<motor_messages::GearMotorMove>;
using MotionControlType =
    motion_controller::MotionController<lms::LeadScrewConfig>;
using PipetteMotionControlType =
    pipette_motion_controller::PipetteMotionController<lms::LeadScrewConfig>;

struct LowThroughputInterruptQueues {
    MoveQueue plunger_queue;
};

struct HighThroughputInterruptQueues {
    MoveQueue plunger_queue;
    GearMoveQueue right_motor_queue;
    GearMoveQueue left_motor_queue;
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
    pipette_motor_hardware_task::PipetteMotorHardwareTask left;
    pipette_motor_hardware_task::PipetteMotorHardwareTask right;
};

}  // namespace gear_motor

}  // namespace interfaces
