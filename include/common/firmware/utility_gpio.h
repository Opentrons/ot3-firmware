#pragma once

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void limit_switch_gpio_init();
void LED_drive_gpio_init();
void utility_gpio_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
