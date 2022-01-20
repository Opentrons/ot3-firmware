#include "head/firmware/adc_comms.hpp"

#include "platform_specific_hal_conf.h"

using namespace adc;
/*
 * Public Functions
 */
ADC::ADC(ADC_interface ADC_intf_instance1, ADC_interface ADC_intf_instance2)
    : ADC_intf1(ADC_intf_instance1), ADC_intf2(ADC_intf_instance2) {}

auto ADC::get_readings() -> adc::RawADCReadings {
    auto voltage_read = adc::RawADCReadings{
        .z_motor = static_cast<uint16_t>(adc_read_z_motor()),
        .a_motor = static_cast<uint16_t>(adc_read_a_motor()),
        .gripper = static_cast<uint16_t>(adc_read_gripper())};
    return voltage_read;
}