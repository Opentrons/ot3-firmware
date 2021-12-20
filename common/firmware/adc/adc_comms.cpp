#include "common/firmware/adc_comms.hpp"

#include "platform_specific_hal_conf.h"

using namespace adc;
/*
 * Public Functions
 */
struct voltage_read tmp = {
    .z_motor = 4294967295, .a_motor = 4294967295, .gripper = 4294967295};
ADC::ADC(ADC_interface ADC_intf_instance1, ADC_interface ADC_intf_instance2)
    : ADC_intf1(ADC_intf_instance1),
      ADC_intf2(ADC_intf_instance2),
      voltage_read(tmp) {}

void ADC::get_readings() { voltage_read = adc_read_voltages(); }