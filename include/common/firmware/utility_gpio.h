#pragma once

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void limit_switch_gpio_init();
void LED_drive_gpio_init();
void turn_on_LED_pin();
void turn_off_LED_pin();
void check_limit_switch();
void utility_gpio_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
