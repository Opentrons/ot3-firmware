#include "pipettes/core/configs.hpp"
#include "pipettes/firmware/interfaces_g4.hpp"

using namespace interfaces;

template <>
auto interfaces::get_interrupt_queues<PipetteType::SINGLE_CHANNEL>()
    -> LowThroughputInterruptQueues {
    return LowThroughputInterruptQueues{
        .plunger_queue = MoveQueue{"Linear Motor Queue"},
        .plunger_update_queue = UpdatePositionQueue{"Linear PUpdate Queue"}};
}

template <>
auto interfaces::get_interrupt_queues<PipetteType::EIGHT_CHANNEL>()
    -> LowThroughputInterruptQueues {
    return LowThroughputInterruptQueues{
        .plunger_queue = MoveQueue{"Linear Motor Queue"},
        .plunger_update_queue = UpdatePositionQueue{"Linear PUpdate Queue"}};
}

template <>
auto interfaces::get_interrupt_queues<PipetteType::NINETY_SIX_CHANNEL>()
    -> HighThroughputInterruptQueues {
    return HighThroughputInterruptQueues{
        .plunger_queue = MoveQueue{"Linear Motor Queue"},
        .plunger_update_queue = UpdatePositionQueue{"Linear Update Queue"},
        .right_motor_queue = GearMoveQueue{"Right Gear Motor Queue"},
        .right_update_queue = UpdatePositionQueue{"Right PUpdate Queue"},
        .left_motor_queue = GearMoveQueue{"Left Gear Motor Queue"},
        .left_update_queue = UpdatePositionQueue{"Left PUpdate Queue"}};
}

template <>
auto interfaces::get_interrupt_queues<PipetteType::THREE_EIGHTY_FOUR_CHANNEL>()
    -> HighThroughputInterruptQueues {
    return HighThroughputInterruptQueues{
        .plunger_queue = MoveQueue{"Linear Motor Queue"},
        .plunger_update_queue = UpdatePositionQueue{"Linear Update Queue"},
        .right_motor_queue = GearMoveQueue{"Right Gear Motor Queue"},
        .right_update_queue = UpdatePositionQueue{"Right PUpdate Queue"},
        .left_motor_queue = GearMoveQueue{"Left Gear Motor Queue"},
        .left_update_queue = UpdatePositionQueue{"Left PUpdate Queue"}};
}

void linear_motor::encoder_interrupt(motor_hardware::MotorHardware& hw,
                                     int32_t direction) {
    hw.encoder_overflow(direction);
}

auto linear_motor::get_interrupt(motor_hardware::MotorHardware& hw,
                                 LowThroughputInterruptQueues& queues,
                                 stall_check::StallCheck& stall)
    -> MotorInterruptHandlerType<linear_motor_tasks::QueueClient> {
    return motor_handler::MotorInterruptHandler(
        queues.plunger_queue, linear_motor_tasks::get_queues(), hw, stall,
        queues.plunger_update_queue);
}

auto linear_motor::get_interrupt(motor_hardware::MotorHardware& hw,
                                 HighThroughputInterruptQueues& queues,
                                 stall_check::StallCheck& stall)
    -> MotorInterruptHandlerType<linear_motor_tasks::QueueClient> {
    return motor_handler::MotorInterruptHandler(
        queues.plunger_queue, linear_motor_tasks::get_queues(), hw, stall,
        queues.plunger_update_queue);
}

auto linear_motor::get_motor_hardware(motor_hardware::HardwareConfig pins)
    -> motor_hardware::MotorHardware {
    return motor_hardware::MotorHardware(pins, &htim7, &htim2);
}

auto linear_motor::get_motion_control(motor_hardware::MotorHardware& hw,
                                      LowThroughputInterruptQueues& queues)
    -> MotionControlType {
    return motion_controller::MotionController{
        configs::linear_motion_sys_config_by_axis(PipetteType::SINGLE_CHANNEL),
        hw,
        motor_messages::MotionConstraints{.min_velocity = 1,
                                          .max_velocity = 2,
                                          .min_acceleration = 1,
                                          .max_acceleration = 2},
        queues.plunger_queue, queues.plunger_update_queue};
}

auto linear_motor::get_motion_control(motor_hardware::MotorHardware& hw,
                                      HighThroughputInterruptQueues& queues)
    -> MotionControlType {
    return motion_controller::MotionController{
        configs::linear_motion_sys_config_by_axis(
            PipetteType::NINETY_SIX_CHANNEL),
        hw,
        motor_messages::MotionConstraints{.min_velocity = 1,
                                          .max_velocity = 2,
                                          .min_acceleration = 1,
                                          .max_acceleration = 2},
        queues.plunger_queue, queues.plunger_update_queue};
}

auto gear_motor::get_interrupts(gear_motor::GearHardware& hw,
                                HighThroughputInterruptQueues& queues,
                                GearStallCheck& stall)
    -> gear_motor::GearInterruptHandlers {
    return gear_motor::GearInterruptHandlers{
        .left = motor_handler::MotorInterruptHandler(
            queues.left_motor_queue, gear_motor_tasks::get_left_gear_queues(),
            hw.left, stall.left, queues.left_update_queue),
        .right = motor_handler::MotorInterruptHandler(
            queues.right_motor_queue, gear_motor_tasks::get_right_gear_queues(),
            hw.right, stall.right, queues.right_update_queue)};
}

auto gear_motor::get_interrupts(gear_motor::UnavailableGearHardware&,
                                LowThroughputInterruptQueues&, GearStallCheck&)
    -> gear_motor::UnavailableGearInterrupts {
    return gear_motor::UnavailableGearInterrupts{};
}

auto gear_motor::get_motor_hardware(
    motor_configs::LowThroughputPipetteMotorHardware)
    -> gear_motor::UnavailableGearHardware {
    return gear_motor::UnavailableGearHardware{};
}

auto gear_motor::get_motor_hardware(
    motor_configs::HighThroughputPipetteMotorHardware pins)
    -> gear_motor::GearHardware {
    return gear_motor::GearHardware{
        .left = pipette_motor_hardware::MotorHardware(pins.left_gear_motor,
                                                      &htim6, &htim2),
        .right = pipette_motor_hardware::MotorHardware(pins.right_gear_motor,
                                                       &htim6, &htim2)};
}

auto gear_motor::get_motor_hardware_tasks(gear_motor::UnavailableGearHardware&)
    -> gear_motor::UnavailableGearHardwareTasks {
    return gear_motor::UnavailableGearHardwareTasks{};
}
auto gear_motor::get_motor_hardware_tasks(gear_motor::GearHardware& hd_ware)
    -> gear_motor::GearMotorHardwareTasks {
    return gear_motor::GearMotorHardwareTasks{
        .left = pipette_motor_hardware_task::PipetteMotorHardwareTask(
            &hd_ware.left, "left gear motor hardware task"),
        .right = pipette_motor_hardware_task::PipetteMotorHardwareTask(
            &hd_ware.right, "right gear motor hardware task")};
}

auto gear_motor::get_motion_control(gear_motor::GearHardware& hw,
                                    HighThroughputInterruptQueues& queues)
    -> gear_motor::GearMotionControl {
    return gear_motor::GearMotionControl{
        .left =
            pipette_motion_controller::PipetteMotionController{
                configs::linear_motion_sys_config_by_axis(
                    PipetteType::NINETY_SIX_CHANNEL),
                hw.left,
                motor_messages::MotionConstraints{.min_velocity = 1,
                                                  .max_velocity = 2,
                                                  .min_acceleration = 1,
                                                  .max_acceleration = 2},
                queues.left_motor_queue},
        .right = pipette_motion_controller::PipetteMotionController{
            configs::linear_motion_sys_config_by_axis(
                PipetteType::NINETY_SIX_CHANNEL),
            hw.right,
            motor_messages::MotionConstraints{.min_velocity = 1,
                                              .max_velocity = 2,
                                              .min_acceleration = 1,
                                              .max_acceleration = 2},
            queues.right_motor_queue}};
}

auto gear_motor::get_motion_control(gear_motor::UnavailableGearHardware&,
                                    LowThroughputInterruptQueues&)
    -> gear_motor::UnavailableGearMotionControl {
    return gear_motor::UnavailableGearMotionControl{};
}

auto gear_motor::gear_callback(gear_motor::GearInterruptHandlers& interrupts)
    -> void {
    interrupts.left.run_interrupt();
    interrupts.right.run_interrupt();
}

auto gear_motor::gear_callback(gear_motor::UnavailableGearInterrupts&) -> void {
}
