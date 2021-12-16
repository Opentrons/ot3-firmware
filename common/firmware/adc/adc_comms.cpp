#include "common/firmware/adc_comms.hpp"

#include "platform_specific_hal_conf.h"
#include "adc.h"

using namespace adc;
/*
 * Public Functions
 */

ADC::ADC(ADC_interface ADC_intf_instance) : ADC_intf(ADC_intf_instance) {}

void ADC::get_readings(ADC_interface ADC_intf_instance) {
}
