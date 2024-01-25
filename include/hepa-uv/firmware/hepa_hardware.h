#pragma once

#include <stdint.h>

#include "hepa-uv/core/constants.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void set_hepa_fan_pwm(uint32_t duty_cycle);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
