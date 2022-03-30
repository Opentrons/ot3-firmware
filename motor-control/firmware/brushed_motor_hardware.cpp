#include "motor-control/firmware/brushed_motor_hardware.hpp"

#include "motor_control_hardware.h"

using namespace motor_hardware;

void BrushedMotorHardware::positive_direction() {
    motor_hardware_start_pwm(pins.pwm_1.tim, pins.pwm_1.channel);
    motor_hardware_stop_pwm(pins.pwm_2.tim, pins.pwm_2.channel);
}

void BrushedMotorHardware::negative_direction() {
    motor_hardware_stop_pwm(pins.pwm_1.tim, pins.pwm_1.channel);
    motor_hardware_start_pwm(pins.pwm_2.tim, pins.pwm_2.channel);
}

void BrushedMotorHardware::activate_motor() {
    motor_hardware_set_pin(pins.enable.port, pins.enable.pin,
                           pins.enable.active_setting);
}
void BrushedMotorHardware::deactivate_motor() {
    motor_hardware_reset_pin(pins.enable.port, pins.enable.pin,
                             pins.enable.active_setting);
}
bool BrushedMotorHardware::check_limit_switch() {
    return motor_hardware_get_pin_value(pins.limit_switch.port,
                                        pins.limit_switch.pin,
                                        pins.limit_switch.active_setting);
}

void BrushedMotorHardware::grip() {
    motor_hardware_set_pin(pins.enable.port, pins.enable.pin,
                           pins.enable.active_setting);
    positive_direction();
}

void BrushedMotorHardware::home() {
    motor_hardware_set_pin(pins.enable.port, pins.enable.pin,
                           pins.enable.active_setting);
    negative_direction();

bool BrushedMotorHardware::check_sync_in() {
    return motor_hardware_get_pin_value(pins.sync_in.port, pins.sync_in.pin,
                                        pins.sync_in.active_setting);
}
