#pragma once

#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stall_check.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/interfaces.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/motor_configurations.hpp"
#include "pipettes/core/motor_interrupt_handler.hpp"
#include "pipettes/core/pipette_type.h"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_timer_hardware.h"
#pragma GCC diagnostic pop

/*
 * (CM, 11/3/2022)
 * This file exists in addition to interfaces.hpp to accommodate the l5 firmware
 * after the changes in the enable-pipette-encoder-output branch. interfaces.hpp
 * can be deleted with the l5 firmware, at which point this should be renamed
 * to interfaces.hpp.
 *
 * */

namespace interfaces {

template <typename Client>
using MotorInterruptHandlerType = motor_handler::MotorInterruptHandler<
    freertos_message_queue::FreeRTOSMessageQueue, Client,
    motor_messages::SensorSyncMove, motor_hardware::MotorHardware>;
template <typename Client>
using GearMotorInterruptHandlerType = motor_handler::MotorInterruptHandler<
    freertos_message_queue::FreeRTOSMessageQueue, Client,
    motor_messages::GearMotorMove, motor_hardware::MotorHardware>;
template <typename Client>
using PipetteMotorInterruptHandlerType = pipettes::PipetteMotorInterruptHandler<
    freertos_message_queue::FreeRTOSMessageQueue, Client,
    motor_messages::SensorSyncMove, motor_hardware::MotorHardware,
    sensor_tasks::QueueClient>;

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

auto get_interrupt(motor_hardware::MotorHardware& hw,
                   LowThroughputInterruptQueues& queues,
                   stall_check::StallCheck& stall,
                   sensor_tasks::QueueClient& sensor_client)
    -> PipetteMotorInterruptHandlerType<linear_motor_tasks::QueueClient>;
auto get_interrupt(motor_hardware::MotorHardware& hw,
                   HighThroughputInterruptQueues& queues,
                   stall_check::StallCheck& stall,
                   sensor_tasks::QueueClient& sensor_client)
    -> PipetteMotorInterruptHandlerType<linear_motor_tasks::QueueClient>;
auto get_motor_hardware(motor_hardware::HardwareConfig pins)
    -> motor_hardware::MotorHardware;
auto get_motion_control(motor_hardware::MotorHardware& hw,
                        LowThroughputInterruptQueues& queues)
    -> MotionControlType;
auto get_motion_control(motor_hardware::MotorHardware& hw,
                        HighThroughputInterruptQueues& queues)
    -> MotionControlType;
void encoder_interrupt(motor_hardware::MotorHardware& hw, int32_t direction);

}  // namespace linear_motor

namespace gear_motor {

struct GearHardware {
    motor_hardware::MotorHardware left;
    motor_hardware::MotorHardware right;
};

struct GearInterruptHandlers {
    GearMotorInterruptHandlerType<gear_motor_tasks::QueueClient> left;
    GearMotorInterruptHandlerType<gear_motor_tasks::QueueClient> right;
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
                    GearStallCheck& stall) -> UnavailableGearInterrupts;

auto get_interrupts(GearHardware& hw, HighThroughputInterruptQueues& queues,
                    GearStallCheck& stall) -> GearInterruptHandlers;

auto get_motion_control(UnavailableGearHardware&,
                        LowThroughputInterruptQueues& queues)
    -> UnavailableGearMotionControl;
auto get_motion_control(GearHardware& hw, HighThroughputInterruptQueues& queues)
    -> GearMotionControl;

auto gear_callback(GearInterruptHandlers& interrupts) -> void;

auto gear_callback(UnavailableGearInterrupts&) -> void;

}  // namespace gear_motor
}  // namespace interfaces
