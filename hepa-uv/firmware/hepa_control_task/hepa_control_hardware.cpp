#include "hepa-uv/firmware/hepa_control_hardware.hpp"

#include "hepa-uv/firmware/hepa_hardware.h"

using namespace hepa_control_hardware;

void HepaControlHardware::set_hepa_fan_speed(uint32_t duty_cycle) {
    set_hepa_fan_pwm(duty_cycle);
}

auto HepaControlHardware::get_hepa_fan_rpm() -> uint16_t {
    return hepa_fan_rpm;
}

void HepaControlHardware::reset_hepa_fan_rpm() { hepa_fan_rpm = 0; }

auto HepaControlHardware::enable_tachometer(bool enable) -> bool {
    return enable_hepa_fan_tachometer(enable);
}

void HepaControlHardware::hepa_fan_rpm_irq(uint16_t rpm) { hepa_fan_rpm = rpm; }