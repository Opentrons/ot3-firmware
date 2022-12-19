#pragma once

#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "pipettes/firmware/pipette_motor_hardware.hpp"

namespace motor_hardware_task {
    
class MotorHardwareTask {
  public:
    MotorHardwareTask(motor_hardware::MotorHardwareIface& hardware) : hardware{hardware} {}
    MotorHardwareTask(const MotorHardwareTask& c) = delete;
    MotorHardwareTask(const MotorHardwareTask&& c) = delete;
    auto operator=(const MotorHardwareTask& c) = delete;
    auto operator=(const MotorHardwareTask&& c) = delete;
    ~MotorHardwareTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()() {
        for (;;) {
            vTaskDelay(1);
            hardware.read_estop_in();
            hardware.read_limit_switch();
            hardware.read_sync_in();
        }
    }


  private:
    motor_hardware::MotorHardwareIface& hardware;
};
}  // namespace motor_hardware_task


namespace pipette_motor_hardware_task {

/**
 * The task entry point.
 */
class PipetteMotorHardwareTask {
  public:
    PipetteMotorHardwareTask(motor_hardware::PipetteStepperMotorHardwareIface& hardware) : hardware{hardware} {}
    PipetteMotorHardwareTask(const PipetteMotorHardwareTask& c) = delete;
    PipetteMotorHardwareTask(const PipetteMotorHardwareTask&& c) = delete;
    auto operator=(const PipetteMotorHardwareTask& c) = delete;
    auto operator=(const PipetteMotorHardwareTask&& c) = delete;
    ~PipetteMotorHardwareTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()() {
        for (;;) {
            vTaskDelay(1);
            hardware.read_estop_in();
            hardware.read_limit_switch();
            hardware.read_sync_in();
            hardware.read_tip_sense();
        }
    }


  private:
    motor_hardware::PipetteStepperMotorHardwareIface& hardware;
};

}  // namespace brushed_motor_hardware_task
