#include "motor-control/firmware/brushed_motor_hardware.hpp"

#include "motor_control_hardware.h"

using namespace motor_hardware;

void BrushedMotorHardware::positive_direction() {}

void BrushedMotorHardware::negative_direction() {}

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
