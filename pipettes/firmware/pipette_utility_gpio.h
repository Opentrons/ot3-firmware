#pragma once
#include <stdbool.h>

#include "pipettes/core/pipette_type.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// in both cases, 1 means present/high and 0 means absent/low

int utility_gpio_tip_present();
int utility_gpio_get_mount_id(PipetteType pipette_type);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
