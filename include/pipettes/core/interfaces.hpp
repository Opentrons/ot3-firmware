#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"

namespace interfaces {

using MoveQueue =
freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>;
using MotionControlType =
motion_controller::MotionController<lms::LeadScrewConfig>;
using PipetteMotionControlType =
pipette_motion_controller::PipetteMotionController<lms::LeadScrewConfig>;

struct LowThroughputInterruptQueues {
    MoveQueue motor_queue;
};

struct HighThroughputInterruptQueues {
    MoveQueue linear_motor_queue;
    MoveQueue right_motor_queue;
    MoveQueue left_motor_queue;
};

namespace gear_motor {

struct GearHardware {
    pipette_motor_hardware::MotorHardware left;
    pipette_motor_hardware::MotorHardware right;
};

struct UnavailableGearHardware {};
struct UnavailableGearMotionControl {};
struct UnavailableGearInterrupts {};

struct GearMotionControl {
    PipetteMotionControlType left;
    PipetteMotionControlType right;
};

}

}