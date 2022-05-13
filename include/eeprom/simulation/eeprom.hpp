#pragma once

#include "i2c/simulation/device.hpp"

namespace eeprom {
namespace simulator {

using namespace i2c::hardware;

class EEProm : public I2CDeviceBase {
  public:
    EEProm() : I2CDeviceBase(0xA0) {}
};

}  // namespace simulator
}  // namespace eeprom