#include "hepa-uv/firmware/hepa_control_hardware.hpp"

#include "hepa-uv/firmware/hepauv_hardware.h"

using namespace hepa_control_hardware;

void HepaControlHardware::set_hepa_fan_speed(uint32_t duty_cycle) {
    set_hepa_fan_pwm(duty_cycle);
}