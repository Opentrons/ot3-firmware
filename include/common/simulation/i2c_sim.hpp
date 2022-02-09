#pragma once

#include <vector>

#include "common/core/i2c.hpp"

namespace sim_i2c {

/**
 * Simulation of eeprom on i2c. Read back whatever was written.
 */
class SimI2C : public i2c::I2CDeviceBase {
  public:
    auto central_transmit(uint8_t *data, uint16_t size, uint16_t dev_address,
                          uint32_t timeout) -> bool final;
    auto central_receive(uint8_t *data, uint16_t size, uint16_t dev_address,
                         uint32_t timeout) -> bool final;

  private:
    std::vector<uint8_t> storage{};
};

}  // namespace sim_i2c