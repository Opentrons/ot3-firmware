#include "head/firmware/adc_comms.hpp"

#include "platform_specific_hal_conf.h"

using namespace adc;

ADCChannel::ADCChannel(ADC_HandleTypeDef* ADC_intf_instance, uint32_t channel)
    : _hardware(ADC_intf_instance), _hardware_channel(channel) {}

auto ADCChannel::get_reading() -> uint16_t {
    return adc_read(_hardware, _hardware_channel);
}
/*
 * Public Functions
 */
ADC::ADC(ADC_HandleTypeDef* ADC_intf_instance1,
         ADC_HandleTypeDef* ADC_intf_instance2)
    : a_channel(ADC_intf_instance2, ADC_CHANNEL_12),
      z_channel(ADC_intf_instance2, ADC_CHANNEL_11),
      gripper_channel(ADC_intf_instance1, ADC_CHANNEL_12) {
    adc_setup(ADC_intf_instance1, ADC_intf_instance2);
}
