#pragma once

#include "i2c/simulation/device.hpp"
#include "sensors/core/fdc1004.hpp"

namespace fdc1004_simulator {
using namespace sensors;
using namespace i2c::hardware;

class FDC1004 : public I2CRegisterMap<uint8_t, uint16_t> {
  public:
    FDC1004()
        : I2CRegisterMap(
              fdc1004::ADDRESS,
              {{static_cast<uint8_t>(fdc1004::Registers::CONF_MEAS1), 0},
               {static_cast<uint8_t>(fdc1004::Registers::FDC_CONF), 0},
               {static_cast<uint8_t>(fdc1004::Registers::MEAS1_MSB), 100},
               {static_cast<uint8_t>(fdc1004::Registers::MEAS1_LSB), 0},
               {static_cast<uint8_t>(fdc1004::Registers::DEVICE_ID),
                fdc1004_utils::DEVICE_ID}}) {}
};

};  // namespace fdc1004_simulator
