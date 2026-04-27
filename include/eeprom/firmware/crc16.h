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

/**
 * Continue accumulating CRC using provided data.
 * @param data Data
 * @param length Length of data
 * @return Accumulated CRC
 */
uint16_t crc16_accumulate(const uint8_t* data, uint8_t length);

/**
 * Reset the accumulated CRC value.
 */
void crc16_reset_accumulator();

void crc16_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
