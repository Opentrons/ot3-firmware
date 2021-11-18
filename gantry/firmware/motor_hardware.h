#pragma once

#include "gantry/core/axis_type.h"
#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern SPI_HandleTypeDef hspi2;

HAL_StatusTypeDef initialize_spi(enum GantryAxisType);
void gantry_driver_CLK_init(enum GantryAxisType);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
