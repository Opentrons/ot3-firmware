#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"

#include "motor-control/firmware/motor_control_hardware.h"

using namespace motor_hardware;

void MotorHardware::step() {
    motor_hardware_set_pin(pins.step.port, pins.step.pin,
                           pins.step.active_setting);
}

void MotorHardware::unstep() {
    motor_hardware_reset_pin(pins.step.port, pins.step.pin,
                             pins.step.active_setting);
}

void MotorHardware::positive_direction() {
    motor_hardware_set_pin(pins.direction.port, pins.direction.pin,
                           pins.direction.active_setting);
}
void MotorHardware::negative_direction() {
    motor_hardware_reset_pin(pins.direction.port, pins.direction.pin,
                             pins.direction.active_setting);
}
void MotorHardware::activate_motor() {
    motor_hardware_set_pin(pins.enable.port, pins.enable.pin,
                           pins.enable.active_setting);
}
void MotorHardware::deactivate_motor() {
    motor_hardware_reset_pin(pins.enable.port, pins.enable.pin,
                             pins.enable.active_setting);
}
void MotorHardware::start_timer_interrupt() {
    motor_hardware_start_timer(tim_handle);
}
void MotorHardware::stop_timer_interrupt() {
    motor_hardware_stop_timer(tim_handle);
}
bool MotorHardware::check_limit_switch() {
    return motor_hardware_get_pin_value(pins.limit_switch.port,
                                        pins.limit_switch.pin,
                                        pins.limit_switch.active_setting);
}

bool MotorHardware::check_sync_in() {
    return motor_hardware_get_pin_value(pins.sync_in.port, pins.sync_in.pin,
                                        pins.sync_in.active_setting);
}

void MotorHardware::set_LED(bool status) {
    if (status) {
        motor_hardware_set_pin(pins.led.port, pins.led.pin,
                               pins.led.active_setting);
    } else {
        motor_hardware_reset_pin(pins.led.port, pins.led.pin,
                                 pins.led.active_setting);
    }
}

uint32_t MotorHardware::get_encoder_pulses() {
    return motor_hardware_encoder_pulse_count(enc_handle);
}

void MotorHardware::reset_encoder_pulses() {
    motor_hardware_reset_encoder_count(enc_handle);
}
