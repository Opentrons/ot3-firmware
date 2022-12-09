#include "motor-control/firmware/brushed_motor/driver_hardware.hpp"
#include "motor-control/firmware/motor_control_hardware.h"

using namespace brushed_motor_driver;

bool BrushedMotorDriver::start_digital_analog_converter() {
    return motor_hardware_start_dac(dac.dac_handle, dac.channel);
}
bool BrushedMotorDriver::stop_digital_analog_converter() {
    return motor_hardware_stop_dac(dac.dac_handle, dac.channel);
}
bool BrushedMotorDriver::set_reference_voltage(float val) {
    conf.vref = val;
    auto vref_val =
        static_cast<uint32_t>(val * DAC_DATA_MULTIPLIER / VOLTAGE_REFERENCE);
    return motor_hardware_set_dac_value(dac.dac_handle, dac.channel,
                                        dac.data_algn, vref_val);
}

void BrushedMotorDriver::setup() {
    set_reference_voltage(conf.vref);
    start_digital_analog_converter();
}