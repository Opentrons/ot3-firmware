#pragma once

#include <cstdint>

#include "common/core/i2c.hpp"
#include "common/firmware/i2c.h"

namespace i2c_comms {

class I2C : public i2c::I2CDeviceBase {
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
    auto central_transmit(i2c::MaxMessageBuffer& buffer, uint16_t dev_address,
                          uint32_t timeout) -> bool final;

    /**
     * Receive data
     * @return True if succeeded
     */
    auto central_receive(i2c::MaxMessageBuffer& buffer, uint16_t dev_address,
                         uint32_t timeout) -> bool final;

    /**
     * Abstract delay function
     * @return Nothing.
     */
    auto wait_during_poll(uint16_t delay) -> void final;

    auto set_handle(HAL_I2C_HANDLE i2c_handle) -> void;

  private:
    HAL_I2C_HANDLE handle = nullptr;
};
}  // namespace i2c