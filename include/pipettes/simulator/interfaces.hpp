#pragma once

#include "motor-control/core/stall_check.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/interfaces.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/motor_configurations.hpp"
#include "pipettes/core/pipette_type.h"

namespace interfaces {

template <typename Client, typename DriverClient>
using MotorInterruptHandlerType = motor_handler::MotorInterruptHandler<
    freertos_message_queue::FreeRTOSMessageQueue, Client, DriverClient,
    motor_messages::Move, sim_motor_hardware_iface::SimMotorHardwareIface>;

template <typename Client>
using GearMotorInterruptHandlerType = motor_handler::MotorInterruptHandler<
    freertos_message_queue::FreeRTOSMessageQueue, Client, Client,
    motor_messages::GearMotorMove,
    sim_motor_hardware_iface::SimGearMotorHardwareIface>;

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
                   LowThroughputInterruptQueues& queues,
                   stall_check::StallCheck& stall)
    -> MotorInterruptHandlerType<
        linear_motor_tasks::QueueClient,
        linear_motor_tasks::tmc2130_driver::QueueClient>;

auto get_interrupt(sim_motor_hardware_iface::SimMotorHardwareIface& hw,
                   HighThroughputInterruptQueues& queues,
                   stall_check::StallCheck& stall)
    -> MotorInterruptHandlerType<
        linear_motor_tasks::QueueClient,
        linear_motor_tasks::tmc2160_driver::QueueClient>;

auto get_interrupt_driver(
    sim_motor_hardware_iface::SimMotorHardwareIface& hw,
    LowThroughputInterruptQueues& queues,
    MotorInterruptHandlerType<linear_motor_tasks::QueueClient,
                              linear_motor_tasks::tmc2130_driver::QueueClient>&
        handler)
    -> motor_interrupt_driver::MotorInterruptDriver<
        linear_motor_tasks::QueueClient,
        linear_motor_tasks::tmc2130_driver::QueueClient, motor_messages::Move,
        sim_motor_hardware_iface::SimMotorHardwareIface>;

auto get_interrupt_driver(
    sim_motor_hardware_iface::SimMotorHardwareIface& hw,
    HighThroughputInterruptQueues& queues,
    MotorInterruptHandlerType<linear_motor_tasks::QueueClient,
                              linear_motor_tasks::tmc2160_driver::QueueClient>&
        handler)
    -> motor_interrupt_driver::MotorInterruptDriver<
        linear_motor_tasks::QueueClient,
        linear_motor_tasks::tmc2160_driver::QueueClient, motor_messages::Move,
        sim_motor_hardware_iface::SimMotorHardwareIface>;

auto get_motor_hardware() -> sim_motor_hardware_iface::SimMotorHardwareIface;

auto get_motion_control(sim_motor_hardware_iface::SimMotorHardwareIface& hw,
                        LowThroughputInterruptQueues& queues)
    -> MotionControlType;
auto get_motion_control(sim_motor_hardware_iface::SimMotorHardwareIface& hw,
                        HighThroughputInterruptQueues& queues)
    -> MotionControlType;

}  // namespace linear_motor

namespace gear_motor {

struct GearInterruptHandlers {
    GearMotorInterruptHandlerType<gear_motor_tasks::QueueClient> left;
    GearMotorInterruptHandlerType<gear_motor_tasks::QueueClient> right;
};

struct GearInterruptDrivers {
    motor_interrupt_driver::MotorInterruptDriver<
        gear_motor_tasks::QueueClient, gear_motor_tasks::QueueClient,
        motor_messages::GearMotorMove,
        sim_motor_hardware_iface::SimGearMotorHardwareIface>
        left;
    motor_interrupt_driver::MotorInterruptDriver<
        gear_motor_tasks::QueueClient, gear_motor_tasks::QueueClient,
        motor_messages::GearMotorMove,
        sim_motor_hardware_iface::SimGearMotorHardwareIface>
        right;
};

struct GearHardware {
    sim_motor_hardware_iface::SimGearMotorHardwareIface left;
    sim_motor_hardware_iface::SimGearMotorHardwareIface right;
};

struct GearStallCheck {
    stall_check::StallCheck left;
    stall_check::StallCheck right;
};

auto get_motor_hardware(motor_configs::LowThroughputPipetteMotorHardware)
    -> UnavailableGearHardware;
auto get_motor_hardware(motor_configs::HighThroughputPipetteMotorHardware pins)
    -> GearHardware;

auto get_motor_hardware_tasks(UnavailableGearHardware&)
    -> UnavailableGearHardwareTasks;
auto get_motor_hardware_tasks(GearHardware& hd_ware) -> GearMotorHardwareTasks;

auto get_interrupts(UnavailableGearHardware&, LowThroughputInterruptQueues&,
                    GearStallCheck&) -> UnavailableGearInterrupts;

auto get_interrupts(GearHardware& hw, HighThroughputInterruptQueues& queues,
                    GearStallCheck& stall) -> GearInterruptHandlers;

auto get_interrupt_drivers(GearInterruptHandlers& interrupts,
                           HighThroughputInterruptQueues& queues,
                           GearHardware& hw) -> GearInterruptDrivers;

auto get_interrupt_drivers(UnavailableGearHardware&,
                           LowThroughputInterruptQueues&, GearHardware&)
    -> UnavailableGearInterrupts;

auto get_motion_control(UnavailableGearHardware&,
                        LowThroughputInterruptQueues& queues)
    -> UnavailableGearMotionControl;
auto get_motion_control(GearHardware& hw, HighThroughputInterruptQueues& queues)
    -> GearMotionControl;

}  // namespace gear_motor
}  // namespace interfaces
