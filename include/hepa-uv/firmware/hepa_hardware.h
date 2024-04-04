#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "hepa-uv/core/constants.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef void (*hepa_rpm_callback)(uint16_t);

void initialize_tachometer(hepa_rpm_callback rpm_callback);
bool enable_hepa_fan_tachometer(bool enable);
void set_hepa_fan_pwm(uint32_t duty_cycle);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
