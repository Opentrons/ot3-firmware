#pragma once

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi2;

HAL_StatusTypeDef initialize_spi(SPI_HandleTypeDef* hspi);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
