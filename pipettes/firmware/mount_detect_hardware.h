#pragma once

#ifdef __cplusplus
#extern "C" {
#endif // defined(__cplusplus)
#include <stdint.h>

void adc_setup();
uint16_t adc_read();

#ifdef __cplusplus
} // extern "C"
#endif // defined(__cplusplus)
