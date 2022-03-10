#include "motor-control/firmware/brushed_motor_hardware.hpp"

#include "motor_control_hardware.h"

using namespace brushed_motor_hardware;

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
bool BrushedMotorHardware::start_digital_analog_converter() {
    return motor_hardware_start_dac(dac.dac_handle, dac.channel);
}
bool BrushedMotorHardware::stop_digital_analog_converter() {
    return motor_hardware_stop_dac(dac.dac_handle, dac.channel);
}
bool BrushedMotorHardware::set_reference_voltage(float val) {
    auto vref_val =
        static_cast<uint32_t>(val * DAC_DATA_MULTIPLIER / VOLTAGE_REFERENCE);
    return motor_hardware_set_dac_value(dac.dac_handle, dac.channel,
                                        dac.data_algn, vref_val);
}