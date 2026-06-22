#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Compute the CRC
 * @param data Data
 * @param length Length of data
 * @return Computed CRC
 */
uint16_t crc16_compute(const uint8_t* data, uint8_t length);

void crc16_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
