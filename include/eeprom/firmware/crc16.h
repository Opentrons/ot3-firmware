#pragma once
#include <stdint.h>

namespace eeprom {

class Crc16 {
  public:
    /**
     * Initialize crc module.
     */
    virutal void crc16_init() = 0;

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
};

/**
 * Initialize crc module.
 */
void crc16_init();

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

}  // namespace eeprom
