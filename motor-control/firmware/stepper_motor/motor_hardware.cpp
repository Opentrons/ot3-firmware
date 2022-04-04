#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "motor-control/firmware/motor_control_hardware.h"
#include "common/firmware/gpio.hpp"

using namespace motor_hardware;

void MotorHardware::step() { gpio::set(pins.step); }

void MotorHardware::unstep() { gpio::reset(pins.step); }

void MotorHardware::positive_direction() { gpio::set(pins.direction); }
void MotorHardware::negative_direction() { gpio::reset(pins.direction); }
void MotorHardware::activate_motor() { gpio::set(pins.enable); }
void MotorHardware::deactivate_motor() { gpio::reset(pins.enable); }
void MotorHardware::start_timer_interrupt() {
    motor_hardware_start_timer(tim_handle);
}
void MotorHardware::stop_timer_interrupt() {
    motor_hardware_stop_timer(tim_handle);
}
bool MotorHardware::check_limit_switch() {
    return gpio::is_set(pins.limit_switch);
}

bool MotorHardware::check_sync_in() { return gpio::is_set(pins.sync_in); }

void MotorHardware::set_LED(bool status) {
    if (status) {
        gpio::set(pins.led);
    } else {
        gpio::reset(pins.led);
        ;
    }
}

uint32_t MotorHardware::get_encoder_pulses() {
    return motor_hardware_encoder_pulse_count(enc_handle);
}

void MotorHardware::reset_encoder_pulses() {
    motor_hardware_reset_encoder_count(enc_handle);
}
