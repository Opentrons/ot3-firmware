#include "common/firmware/adc_comms.hpp"

#include "platform_specific_hal_conf.h"

using namespace adc;
/*
 * Public Functions
 */

ADC::ADC(ADC_interface ADC_intf_instance1, ADC_interface ADC_intf_instance2)
    : ADC_intf1(ADC_intf_instance1),
      ADC_intf2(ADC_intf_instance2),
      voltage_read(0) {}

void ADC::get_readings() { voltage_read = adc_read_voltages(); }