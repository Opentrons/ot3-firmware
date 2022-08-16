#pragma once

#include "i2c/simulation/device.hpp"
#include "sensors/core/fdc1004.hpp"

namespace fdc1004_simulator {
using namespace sensors;
using namespace i2c::hardware;

class FDC1004 : public I2CRegisterMap<uint8_t, uint16_t> {
  public:
    FDC1004()
        : I2CRegisterMap(fdc1004::ADDRESS,
                         {{fdc1004::CONFIGURATION_MEASUREMENT, 0},
                          {fdc1004::FDC_CONFIGURATION, 0},
                          {fdc1004::MSB_MEASUREMENT_1, 100},
                          {fdc1004::LSB_MEASUREMENT_1, 0},
                          {fdc1004::DEVICE_ID_REGISTER, fdc1004::DEVICE_ID}}) {}
};

};  // namespace fdc1004_simulator
