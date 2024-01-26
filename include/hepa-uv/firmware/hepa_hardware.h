#pragma once

#include <stdint.h>

#include "hepa-uv/core/constants.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define HEPA_TACHOMETER_TIMER_FREQ (20000U)

void initialize_tachometer();
void set_hepa_fan_pwm(uint32_t duty_cycle);
uint32_t get_hepa_fan_rpm();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
