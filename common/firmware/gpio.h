#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void gpio_set(void* port, uint16_t pin, uint8_t active_setting);

void gpio_reset(void* port, uint16_t pin, uint8_t active_setting);

bool gpio_is_set(void* port, uint16_t pin, uint8_t active_setting);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
