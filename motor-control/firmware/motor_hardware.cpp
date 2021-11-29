#include "motor-control/firmware/motor_hardware.hpp"
#include "motor_control_hardware.h"

using namespace motor_hardware;

void MotorHardware::step() {
    motor_hardware_set_pin(pins.step.port, pins.step.pin, pins.step.active_setting);
}

void MotorHardware::unstep() {
    motor_hardware_reset_pin(pins.step.port, pins.step.pin, pins.step.active_setting);
}

void MotorHardware::positive_direction() {
    motor_hardware_set_pin(pins.direction.port, pins.direction.pin, pins.direction.active_setting);
}
void MotorHardware::negative_direction() {
    motor_hardware_reset_pin(pins.direction.port, pins.direction.pin, pins.direction.active_setting);
}
void MotorHardware::activate_motor() {
    motor_hardware_set_pin(pins.enable.port, pins.enable.pin, pins.enable.active_setting);
}
void MotorHardware::deactivate_motor() {
    motor_hardware_reset_pin(pins.enable.port, pins.enable.pin, pins.enable.active_setting);
}
void MotorHardware::start_timer_interrupt() {
    motor_hardware_start_timer(tim_handle);
}
void MotorHardware::stop_timer_interrupt() {
    motor_hardware_stop_timer(tim_handle);
}
