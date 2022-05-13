#pragma once

#include "sensors/core/fdc1004.hpp"
#include "i2c/simulation/device.hpp"

namespace fdc1004_simulator {
using namespace sensors;
using namespace i2c::hardware;

class FDC1004 : public I2CRegisterMap<uint8_t, uint16_t>  {
  public:
    FDC1004(): I2CRegisterMap(fdc1004::ADDRESS, REGISTERS) {
    }

  private:
    static const FDC1004::BackingMap REGISTERS;
};


const FDC1004::BackingMap FDC1004::REGISTERS = {{fdc1004::CONFIGURATION_MEASUREMENT, 0},
 {fdc1004::FDC_CONFIGURATION, 0},
 {fdc1004::MSB_MEASUREMENT_1, 2000},
 {fdc1004::LSB_MEASUREMENT_1, 200},
 {fdc1004::DEVICE_ID_REGISTER, fdc1004::DEVICE_ID}};

};  // namespace fdc1004_simulator
