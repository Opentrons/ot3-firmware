#pragma once

#include "platform_specific_hal_conf.h"
#include "pipettes/core/pipette_type.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern TIM_HandleTypeDef htim2;

void initialize_enc(PipetteType pipette_type);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
