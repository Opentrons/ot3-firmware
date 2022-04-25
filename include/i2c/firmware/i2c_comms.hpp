#pragma once

#include <cstdint>

#include "i2c/core/hardware_iface.hpp"
#include "i2c/firmware/i2c.h"

namespace i2c {
namespace hardware {
class I2C : public I2CDeviceBase {
  public:
    explicit I2C() = default;
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

    auto set_handle(HAL_I2C_HANDLE i2c_handle) -> void;

  private:
    HAL_I2C_HANDLE handle = nullptr;
};
};  // namespace hardware
};  // namespace i2c
