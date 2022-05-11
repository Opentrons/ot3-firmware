#include "motor-control/firmware/brushed_motor/brushed_motor_hardware.hpp"

#include "common/firmware/gpio.hpp"
#include "motor-control/firmware/motor_control_hardware.h"

using namespace motor_hardware;

void BrushedMotorHardware::positive_direction() {
    motor_hardware_start_pwm(pins.pwm_1.tim, pins.pwm_1.channel);
    motor_hardware_stop_pwm(pins.pwm_2.tim, pins.pwm_2.channel);
}

void BrushedMotorHardware::negative_direction() {
    motor_hardware_stop_pwm(pins.pwm_1.tim, pins.pwm_1.channel);
    motor_hardware_start_pwm(pins.pwm_2.tim, pins.pwm_2.channel);
}

void BrushedMotorHardware::start_timer_interrupt() {
    motor_hardware_start_timer(pins.pwm_1.tim);  // start base timer
}
void BrushedMotorHardware::stop_timer_interrupt() {
    motor_hardware_stop_timer(pins.pwm_1.tim);  // stop base timer
}

void BrushedMotorHardware::activate_motor() { gpio::set(pins.enable); }
void BrushedMotorHardware::deactivate_motor() { gpio::reset(pins.enable); }

bool BrushedMotorHardware::check_limit_switch() {
    return gpio::is_set(pins.limit_switch);
}

void BrushedMotorHardware::grip() { positive_direction(); }

void BrushedMotorHardware::ungrip() { negative_direction(); }

void BrushedMotorHardware::stop_pwm() {
    motor_hardware_stop_pwm(pins.pwm_1.tim, pins.pwm_1.channel);
    motor_hardware_stop_pwm(pins.pwm_2.tim, pins.pwm_2.channel);
}

bool BrushedMotorHardware::check_sync_in() {
    return gpio::is_set(pins.limit_switch);
}

uint32_t BrushedMotorHardware::get_encoder_pulses() {
    return motor_hardware_encoder_pulse_count(enc_handle);
}

void BrushedMotorHardware::reset_encoder_pulses() {
    motor_hardware_reset_encoder_count(enc_handle);
}

void BrushedMotorHardware::clear_encoder_SR() {
    motor_hardware_clear_status_register(enc_handle);
}

bool BrushedMotorHardware::get_encoder_SR_flag() {
    return motor_hardware_encoder_get_status_register(enc_handle);
}

bool BrushedMotorHardware::get_encoder_direction() {
    return motor_hardware_encoder_get_direction(enc_handle);
}
