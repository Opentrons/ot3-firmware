#pragma once

#include <vector>

#include "common/core/i2c.hpp"

namespace sim_i2c {

/**
 * Simulation of eeprom on i2c. Read back whatever was written.
 */
class SimEEProm : public i2c::I2CDeviceBase {
  public:
    auto master_transmit(uint16_t dev_address, uint8_t *data, uint16_t size,
                         uint32_t timeout) -> bool final;
    auto master_receive(uint16_t dev_address, uint8_t *data, uint16_t size,
                        uint32_t timeout) -> bool final;

  private:
    std::vector<uint8_t> data{};
};

}  // namespace sim_i2c