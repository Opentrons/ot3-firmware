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
    I2CDeviceBase(const I2CDeviceBase&) = default;
    auto operator=(const I2CDeviceBase&) -> I2CDeviceBase& = default;
    I2CDeviceBase(I2CDeviceBase&&) = default;
    auto operator=(I2CDeviceBase&&) -> I2CDeviceBase& = default;

    /**
     * Abstract transmit function
     * @return True if succeeded
     */
    virtual auto master_transmit(uint16_t dev_address, uint8_t* data,
                                 uint16_t size, uint32_t timeout) -> bool = 0;

    /**
     * Abstract receive function
     * @return True if succeeded
     */
    virtual auto master_receive(uint16_t dev_address, uint8_t* data,
                                uint16_t size, uint32_t timeout) -> bool = 0;
};

}  // namespace i2c
