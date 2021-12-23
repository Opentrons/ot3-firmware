#pragma once

#include <cstdint>

namespace i2c {

/**
 * Abstract i2c device.
 */
class I2CDeviceBase {
  public:
    I2CDeviceBase() = default;
    virtual ~I2CDeviceBase() = default;

    /**
     * Abstract transmit function
     * @return True if succeeded
     */
    virtual bool master_transmit(uint16_t DevAddress, uint8_t *data, uint16_t size, uint32_t timeout) = 0;

    /**
     * Abstract receive function
     * @return True if succeeded
     */
    virtual bool master_receive(uint16_t DevAddress, uint8_t *data, uint16_t size, uint32_t timeout) = 0;
};

}

