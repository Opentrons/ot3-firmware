#pragma once

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern SPI_HandleTypeDef hspi2;
HAL_StatusTypeDef initialize_spi(void);
void motor_driver_CLK_gpio_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
