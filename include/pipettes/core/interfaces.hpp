#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"

namespace interfaces {

using MoveQueue =
    freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>;
using GearMoveQueue =
    freertos_message_queue::FreeRTOSMessageQueue<motor_messages::GearMotorMove>;
using MotionControlType =
    motion_controller::MotionController<lms::LeadScrewConfig>;
using PipetteMotionControlType =
    pipette_motion_controller::PipetteMotionController<lms::LeadScrewConfig>;
using UpdatePositionQueue = freertos_message_queue::FreeRTOSMessageQueue<
    can::messages::UpdateMotorPositionRequest>;

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

struct GearMotionControl {
    PipetteMotionControlType left;
    PipetteMotionControlType right;
};

}  // namespace gear_motor

}  // namespace interfaces