#pragma once

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim7;

typedef void (*motor_interrupt_callback)();
void initialize_timer(motor_interrupt_callback);
HAL_StatusTypeDef initialize_spi(void);
void motor_driver_CLK_gpio_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
