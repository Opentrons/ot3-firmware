#pragma once
#include <stdint.h>

/**
 * Initialize crc module.
 */
void crc32_init();

/**
 * Compute the CRC
 * @param data Data
 * @param length Length of data
 * @return Computed CRC
 */
uint32_t crc32_compute(const uint8_t* data, uint8_t length);

/**
 * Continue accumulating CRC using provided data.
 * @param data Data
 * @param length Length of data
 * @return Accumulated CRC
 */
uint32_t crc32_accumulate(const uint8_t* data, uint8_t length);

/**
 * Reset the accumulated CRC value.
 */
void crc32_reset_accumulator();
