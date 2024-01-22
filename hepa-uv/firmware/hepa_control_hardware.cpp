#include "hepa-uv/firmware/hepa_control_hardware.hpp"
#include "hepa-uv/firmware/hardware.h"

using namespace hepa_control_hardware;


void HepaControlHardware::set_hepa_fan_speed(uint32_t duty_cycle) {
    hepa_fan_hw_update_pwm(duty_cycle);
}