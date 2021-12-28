#pragma once

#include "common/core/i2c.hpp"
#include <vector>

namespace i2c {

class SimI2C : public I2CDeviceBase {
  public:
    auto master_transmit(uint16_t dev_address, uint8_t *data, uint16_t size,
                         uint32_t timeout) -> bool final;
    auto master_receive(uint16_t dev_address, uint8_t *data, uint16_t size,
                        uint32_t timeout) -> bool final;
  private:
    std::vector<uint8_t> data{};
};

}  // namespace i2c