#pragma once

#include "i2c/simulation/device.hpp"
#include "eeprom/core/types.hpp"

namespace eeprom {
namespace simulator {

using namespace i2c::hardware;

class EEProm : public I2CDeviceBase {
  public:
    EEProm() : I2CDeviceBase(types::DEVICE_ADDRESS) {}

    auto handle_write(uint8_t *, uint16_t) -> bool {
        return true;
    }

    auto handle_read(uint8_t *, uint16_t ) -> bool  {
        return true;
    }
};

}  // namespace simulator
}  // namespace eeprom