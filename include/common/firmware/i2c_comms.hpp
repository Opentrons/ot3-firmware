#pragma once

#include <array>
#include <cstdint>

#include "common/core/i2c.hpp"
#include "common/firmware/i2c.h"

namespace i2c {
class I2C : public I2CDeviceBase {
  public:
    I2C(HAL_I2C_HANDLE handle);
    ~I2C() = default;
    I2C(const I2C&) = delete;
    I2C(const I2C&&) = delete;
    auto operator=(const I2C&) = delete;
    auto operator=(const I2C&&) = delete;

    /**
     * Transmit data.
     * @return True if succeeded
     */
    bool master_transmit(uint16_t dev_address, uint8_t *data, uint16_t size, uint32_t timeout) final;

    /**
     * Receive data
     * @return True if succeeded
     */
    bool master_receive(uint16_t dev_address, uint8_t *data, uint16_t size, uint32_t timeout) final;

  private:
    HAL_I2C_HANDLE handle;
};
}  // namespace i2c
