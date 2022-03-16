#pragma once

#include <cstdint>

#include "common/core/i2c.hpp"
#include "common/firmware/i2c.h"

namespace i2c {

class I2C : public I2CDeviceBase {
  public:
    explicit I2C();
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

    /**
     * Abstract delay function
     * @return Nothing.
     */
    auto wait_during_poll(uint16_t delay) -> void final;

    auto set_handler(HAL_I2C_HANDLE i2c_handle) -> void;

  private:
    HAL_I2C_HANDLE handle = nullptr;
};
}  // namespace i2c