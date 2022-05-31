#include "pipettes/firmware/interfaces.hpp"

#include "pipettes/core/configs.hpp"

using namespace interfaces;

template <>
auto interfaces::get_interrupt_queues<PipetteType::SINGLE_CHANNEL>()
    -> LowThroughputInterruptQueues {
    return LowThroughputInterruptQueues{.motor_queue =
                                            MoveQueue{"Linear Motor Queue"}};
}

template <>
auto interfaces::get_interrupt_queues<PipetteType::EIGHT_CHANNEL>()
    -> LowThroughputInterruptQueues {
    return LowThroughputInterruptQueues{.motor_queue =
                                            MoveQueue{"Linear Motor Queue"}};
}

template <>
auto interfaces::get_interrupt_queues<PipetteType::NINETY_SIX_CHANNEL>()
    -> HighThroughputInterruptQueues {
    return HighThroughputInterruptQueues{
        .linear_motor_queue = MoveQueue{"Linear Motor Queue"},
        .right_motor_queue = GearMoveQueue{"Right Gear Motor Queue"},
        .left_motor_queue = GearMoveQueue{"Left Gear Motor Queue"}

    };
}

template <>
auto interfaces::get_interrupt_queues<PipetteType::THREE_EIGHTY_FOUR_CHANNEL>()
    -> HighThroughputInterruptQueues {
    return HighThroughputInterruptQueues{
        .linear_motor_queue = MoveQueue{"Linear Motor Queue"},
        .right_motor_queue = GearMoveQueue{"Right Gear Motor Queue"},
        .left_motor_queue = GearMoveQueue{"Left Gear Motor Queue"}};
}

auto linear_motor::get_interrupt(pipette_motor_hardware::MotorHardware& hw,
                                 LowThroughputInterruptQueues& queues)
    -> MotorInterruptHandlerType<linear_motor_tasks::QueueClient> {
    return motor_handler::MotorInterruptHandler(
        queues.motor_queue, linear_motor_tasks::get_queues(), hw);
}

auto linear_motor::get_interrupt(pipette_motor_hardware::MotorHardware& hw,
                                 HighThroughputInterruptQueues& queues)
    -> MotorInterruptHandlerType<linear_motor_tasks::QueueClient> {
    return motor_handler::MotorInterruptHandler(
        queues.linear_motor_queue, linear_motor_tasks::get_queues(), hw);
}

auto linear_motor::get_motor_hardware(
    motor_configs::LowThroughputPipetteMotorHardware pins)
    -> pipette_motor_hardware::MotorHardware {
    return pipette_motor_hardware::MotorHardware(pins.linear_motor, &htim7,
                                                 &htim2);
}

auto linear_motor::get_motor_hardware(
    motor_configs::HighThroughputPipetteMotorHardware pins)
    -> pipette_motor_hardware::MotorHardware {
    return pipette_motor_hardware::MotorHardware(pins.linear_motor, &htim7,
                                                 &htim2);
}

auto linear_motor::get_motion_control(pipette_motor_hardware::MotorHardware hw,
                                      LowThroughputInterruptQueues& queues)
    -> MotionControlType {
    return motion_controller::MotionController{
        configs::linear_motion_sys_config_by_axis(PipetteType::SINGLE_CHANNEL),
        hw,
        motor_messages::MotionConstraints{.min_velocity = 1,
                                          .max_velocity = 2,
                                          .min_acceleration = 1,
                                          .max_acceleration = 2},
        queues.motor_queue};
}

auto linear_motor::get_motion_control(pipette_motor_hardware::MotorHardware hw,
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
        queues.linear_motor_queue};
}

auto gear_motor::get_interrupts(gear_motor::GearHardware& hw,
                                HighThroughputInterruptQueues& queues)
    -> gear_motor::GearInterruptHandlers {
    return gear_motor::GearInterruptHandlers{
        .left = motor_handler::MotorInterruptHandler(
            queues.left_motor_queue, gear_motor_tasks::get_left_gear_queues(),
            hw.left),
        .right = motor_handler::MotorInterruptHandler(
            queues.right_motor_queue, gear_motor_tasks::get_right_gear_queues(),
            hw.right)};
}

auto gear_motor::get_interrupts(gear_motor::UnavailableGearHardware&,
                                LowThroughputInterruptQueues&)
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

auto gear_motor::get_motion_control(gear_motor::GearHardware hw,
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
