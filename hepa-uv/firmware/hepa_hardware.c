#include "hepa-uv/firmware/hepa_hardware.h"
#include "timer_hardware.h"

static uint32_t clamp(uint32_t val, uint32_t min, uint32_t max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void set_hepa_fan_pwm(uint32_t duty_cycle) {
    // calculate the pulse width from the duty cycle
    htim3.Instance->CCR1 = clamp(duty_cycle, 0, 100) * htim3.Init.Period / 100;
}
