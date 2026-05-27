#pragma once
#include <cstdint>

namespace eeprom {

class CRC16Base {
  public:
    virtual ~CRC16Base() = default;
    /**
     * Initialize crc module.
     */
    virtual void crc16_init() = 0;

    /**
     * Compute the CRC
     * @param data Data
     * @param length Length of data
     * @return Computed CRC
     */
    virtual uint16_t crc16_compute(const uint8_t* data, uint8_t length) = 0;

    /**
     * Continue accumulating CRC using provided data.
     * @param data Data
     * @param length Length of data
     * @return Accumulated CRC
     */
    virtual uint16_t crc16_accumulate(const uint8_t* data, uint8_t length) = 0;

    /**
     * Reset the accumulated CRC value.
     */
    virtual void crc16_reset_accumulator() = 0;
};

class CRC16Accelerated : public CRC16Base {
  public:
    /**
     * Initialize crc module.
     */
    void crc16_init() override;

    /**
     * Compute the CRC
     * @param data Data
     * @param length Length of data
     * @return Computed CRC
     */
    uint16_t crc16_compute(const uint8_t* data, uint8_t length) override;

    /**
     * Continue accumulating CRC using provided data.
     * @param data Data
     * @param length Length of data
     * @return Accumulated CRC
     */
    uint16_t crc16_accumulate(const uint8_t* data, uint8_t length) override;

    /**
     * Reset the accumulated CRC value.
     */
    void crc16_reset_accumulator() override;
};

}  // namespace eeprom
