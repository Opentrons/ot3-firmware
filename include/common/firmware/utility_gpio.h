#pragma once

#include "platform_specific_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void limit_switch_gpio_init(void);
void LED_drive_gpio_init(void);
void turn_on_LED_pin(void);
void turn_off_LED_pin(void);

void utility_gpio_init(void);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
