#pragma once

#include <stdint.h>

#include "hepa-uv/core/constants.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void hepa_fan_hw_update_pwm(uint32_t duty_cycle);
void initialize_hardware();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
