#pragma once

#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "pipettes/firmware/pipette_motor_hardware.hpp"

#define TASK_STACK_DEPTH 512

namespace motor_hardware_task {

class MotorHardwareTask {
  public:
    MotorHardwareTask(motor_hardware::MotorHardwareIface* hardware,
                      const char* task_name) : hardware{hardware}, task_name{task_name} {}
    MotorHardwareTask(const MotorHardwareTask& c) = delete;
    MotorHardwareTask(const MotorHardwareTask&& c) = delete;
    auto operator=(const MotorHardwareTask& c) = delete;
    auto operator=(const MotorHardwareTask&& c) = delete;
    ~MotorHardwareTask() = default;
    
    void start_task() {
        xTaskCreateStatic(this->task_function, task_name, TASK_STACK_DEPTH, hardware, 6,
                        backing.data(), &static_task);
    }
    
    /**
     * Task entry point.
     */
    [[noreturn]] static void task_function(void *hw_ptr) {
        motor_hardware::MotorHardwareIface* hardware = reinterpret_cast<motor_hardware::MotorHardwareIface*>(hw_ptr);
        for (;;) {
            vTaskDelay(1);
            hardware->read_estop_in();
            hardware->read_limit_switch();
            hardware->read_sync_in();
        }
    }


  private:
    motor_hardware::MotorHardwareIface* hardware;
    const char* task_name;
    std::array<StackType_t, TASK_STACK_DEPTH> backing{};
    StaticTask_t static_task{};
};
}  // namespace motor_hardware_task


namespace pipette_motor_hardware_task {

/**
 * The task entry point.
 */
class PipetteMotorHardwareTask {
  public:
    PipetteMotorHardwareTask(motor_hardware::PipetteStepperMotorHardwareIface* hardware,
                      const char* task_name) : hardware{hardware}, task_name{task_name} {}
    PipetteMotorHardwareTask(const PipetteMotorHardwareTask& c) = delete;
    PipetteMotorHardwareTask(const PipetteMotorHardwareTask&& c) = delete;
    auto operator=(const PipetteMotorHardwareTask& c) = delete;
    auto operator=(const PipetteMotorHardwareTask&& c) = delete;
    ~PipetteMotorHardwareTask() = default;
    
    void start_task() {
        xTaskCreateStatic(this->task_function, task_name, TASK_STACK_DEPTH, hardware, 6,
                        backing.data(), &static_task);
    }
    
    /**
     * Task entry point.
     */
     [[noreturn]] static void task_function(void *hw_ptr) {
        motor_hardware::PipetteStepperMotorHardwareIface* hardware = reinterpret_cast<motor_hardware::PipetteStepperMotorHardwareIface*>(hw_ptr);
        for (;;) {
            vTaskDelay(1);
            hardware->read_estop_in();
            hardware->read_limit_switch();
            hardware->read_sync_in();
            hardware->read_tip_sense();
        }
    }


  private:
    motor_hardware::PipetteStepperMotorHardwareIface* hardware;
    const char* task_name;
    std::array<StackType_t, TASK_STACK_DEPTH> backing{};
    StaticTask_t static_task{};
};

}  // namespace pipette_motor_hardware_task
