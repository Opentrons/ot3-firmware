#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/motor_configurations.hpp"
#include "pipettes/core/pipette_type.h"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"

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
    MoveQueue plunger_queue;
};

struct HighThroughputInterruptQueues {
    MoveQueue plunger_queue;
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


auto get_interrupt(sim_motor_hardware_iface::SimMotorHardwareIface& hw,
                   MoveQueue& queue)
-> MotorInterruptHandlerType<linear_motor_tasks::QueueClient>;

auto get_interrupt_driver(sim_motor_hardware_iface::SimMotorHardwareIface& hw,
                          MoveQueue& queue,
                          MotorInterruptHandlerType<linear_motor_tasks::QueueClient>& handler
) -> motor_interrupt_driver::MotorInterruptDriver<linear_motor_tasks::QueueClient>;

auto get_motor_hardware()
-> sim_motor_hardware_iface::SimMotorHardwareIface;

auto get_motion_control(sim_motor_hardware_iface::SimMotorHardwareIface hw,
                        LowThroughputInterruptQueues& queues)
-> MotionControlType;
auto get_motion_control(sim_motor_hardware_iface::SimMotorHardwareIface hw,
                        HighThroughputInterruptQueues& queues)
-> MotionControlType;

}  // namespace linear_motor

namespace gear_motor {

struct GearInterruptHandlers {
    MotorInterruptHandlerType<gear_motor_tasks::QueueClient> left;
    MotorInterruptHandlerType<gear_motor_tasks::QueueClient> right;
};

struct GearInterruptDrivers {
    motor_interrupt_driver::GearMotorInterruptDriver<gear_motor_tasks::QueueClient> left;
    motor_interrupt_driver::GearMotorInterruptDriver<gear_motor_tasks::QueueClient> right;
};

struct GearHardware {
    sim_motor_hardware_iface::SimGearMotorHardwareIface left;
    sim_motor_hardware_iface::SimGearMotorHardwareIface right;
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

auto get_interrupt_drivers(GearInterruptHandlers& interrupts,
                           HighThroughputInterruptQueues& queues,
                           GearHardware& hw)
-> GearInterruptDrivers;

auto get_interrupt_drivers(UnavailableGearHardware&,
                                       LowThroughputInterruptQueues&,
                                       GearHardware&)
-> UnavailableGearInterrupts;

auto get_motion_control(UnavailableGearHardware&,
                        LowThroughputInterruptQueues& queues)
-> UnavailableGearMotionControl;
auto get_motion_control(GearHardware hw, HighThroughputInterruptQueues& queues)
-> GearMotionControl;

}  // namespace gear_motor
}  // namespace interfaces