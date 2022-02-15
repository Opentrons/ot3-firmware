#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // defined(__cplusplus)

void adc_init(void);
uint16_t adc_read(void);

#ifdef __cplusplus
} // extern "C"
#endif // defined(__cplusplus)
