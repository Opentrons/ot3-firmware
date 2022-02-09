#pragma once

#include <array>
#include <cstdint>

#include "common/core/i2c.hpp"
#include "common/firmware/i2c.h"

namespace i2c {

class I2C : public I2CDeviceBase {
  public:
    I2C(HAL_I2C_HANDLE handle);
    ~I2C() final = default;
    I2C(const I2C &) = delete;
    I2C(const I2C &&) = delete;
    auto operator=(const I2C &) = delete;
    auto operator=(const I2C &&) = delete;

    /**
     * Transmit data.
     * @return True if succeeded
     */
    auto central_transmit(uint8_t *data, uint16_t size, uint16_t dev_address,
                          uint32_t timeout) -> bool final;

    /**
     * Receive data
     * @return True if succeeded
     */
    auto central_receive(uint8_t *data, uint16_t size, uint16_t dev_address,
                         uint32_t timeout) -> bool final;

  private:
    HAL_I2C_HANDLE handle;
};
}  // namespace i2c