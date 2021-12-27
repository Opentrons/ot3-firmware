#pragma once

#include "common/core/i2c.hpp"

namespace i2c {

class SimI2C : I2CDeviceBase {
  public:
    bool master_transmit(uint16_t dev_address, uint8_t *data, uint16_t size, uint32_t timeout) final;
    bool master_receive(uint16_t dev_address, uint8_t *data, uint16_t size, uint32_t timeout) final;

};


}