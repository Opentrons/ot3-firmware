#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/motor_configurations.hpp"
#include "pipettes/core/pipette_type.h"
#include "pipettes/firmware/pipette_motor_hardware.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_encoder_hardware.h"
#include "motor_timer_hardware.h"
#pragma GCC diagnostic pop

namespace interfaces {
using MoveQueue =
    freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>;
using MotionControlType =
    motion_controller::MotionController<lms::LeadScrewConfig>;
using PipetteMotionControlType =
    pipette_motion_controller::PipetteMotionController<lms::LeadScrewConfig>;

template <typename Client>
using MotorInterruptHandlerType = motor_handler::MotorInterruptHandler<
    freertos_message_queue::FreeRTOSMessageQueue, Client>;

struct LowThroughputInterruptQueues {
    MoveQueue motor_queue;
};

struct HighThroughputInterruptQueues {
    MoveQueue linear_motor_queue;
    MoveQueue right_motor_queue;
    MoveQueue left_motor_queue;
};

template <PipetteType P>
auto get_interrupt_queues()
    -> std::enable_if_t<P == PipetteType::SINGLE_CHANNEL,
                        LowThroughputInterruptQueues>;

template <PipetteType P>
auto get_interrupt_queues() -> std::enable_if_t<P == PipetteType::EIGHT_CHANNEL,
                                                LowThroughputInterruptQueues>;

template <PipetteType P>
auto get_interrupt_queues()
    -> std::enable_if_t<P == PipetteType::NINETY_SIX_CHANNEL,
                        HighThroughputInterruptQueues>;

template <PipetteType P>
auto get_interrupt_queues()
    -> std::enable_if_t<P == PipetteType::THREE_EIGHTY_FOUR_CHANNEL,
                        HighThroughputInterruptQueues>;

template <>
auto get_interrupt_queues<PipetteType::SINGLE_CHANNEL>()
    -> LowThroughputInterruptQueues;

template <>
auto get_interrupt_queues<PipetteType::EIGHT_CHANNEL>()
    -> LowThroughputInterruptQueues;

template <>
auto get_interrupt_queues<PipetteType::NINETY_SIX_CHANNEL>()
    -> HighThroughputInterruptQueues;

template <>
auto get_interrupt_queues<PipetteType::THREE_EIGHTY_FOUR_CHANNEL>()
    -> HighThroughputInterruptQueues;

namespace linear_motor {

auto get_interrupt(pipette_motor_hardware::MotorHardware& hw,
                   LowThroughputInterruptQueues& queues)
    -> MotorInterruptHandlerType<linear_motor_tasks::QueueClient>;
auto get_interrupt(pipette_motor_hardware::MotorHardware& hw,
                   HighThroughputInterruptQueues& queues)
    -> MotorInterruptHandlerType<linear_motor_tasks::QueueClient>;
auto get_motor_hardware(motor_configs::LowThroughputPipetteMotorHardware pins)
    -> pipette_motor_hardware::MotorHardware;
auto get_motor_hardware(motor_configs::HighThroughputPipetteMotorHardware pins)
    -> pipette_motor_hardware::MotorHardware;
auto get_motion_control(pipette_motor_hardware::MotorHardware hw,
                        LowThroughputInterruptQueues& queues)
    -> MotionControlType;
auto get_motion_control(pipette_motor_hardware::MotorHardware hw,
                        HighThroughputInterruptQueues& queues)
    -> MotionControlType;

}  // namespace linear_motor

namespace gear_motor {

struct GearInterruptHandlers {
    MotorInterruptHandlerType<gear_motor_tasks::QueueClient> left;
    MotorInterruptHandlerType<gear_motor_tasks::QueueClient> right;
};

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

auto get_motor_hardware(motor_configs::LowThroughputPipetteMotorHardware)
    -> UnavailableGearHardware;
auto get_motor_hardware(motor_configs::HighThroughputPipetteMotorHardware pins)
    -> GearHardware;

auto get_interrupts(UnavailableGearHardware&, LowThroughputInterruptQueues&)
    -> UnavailableGearInterrupts;

auto get_interrupts(GearHardware& hw, HighThroughputInterruptQueues& queues)
    -> GearInterruptHandlers;

auto get_motion_control(UnavailableGearHardware&,
                        LowThroughputInterruptQueues& queues)
    -> UnavailableGearMotionControl;
auto get_motion_control(GearHardware hw, HighThroughputInterruptQueues& queues)
    -> GearMotionControl;

auto gear_callback(GearInterruptHandlers& interrupts) -> void;

auto gear_callback(UnavailableGearInterrupts&) -> void;

}  // namespace gear_motor
}  // namespace interfaces