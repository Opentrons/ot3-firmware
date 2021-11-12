#pragma once
#include <stdint.h>

#include "gantry/core/axis_type.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void Gantry_Driver_CLK_init(enum GantryAxisType gantry_axis);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus