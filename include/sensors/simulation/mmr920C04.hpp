#pragma once

#include "sensors/core/mmr920C04.hpp"
#include "i2c/simulation/device.hpp"

namespace mmr920C04_simulator {

using namespace sensors;
using namespace i2c::hardware;

class MMR920C04 : public I2CRegisterMap<uint8_t, uint32_t> {
  public:
    MMR920C04(): I2CRegisterMap{mmr920C04::ADDRESS, REGISTERS} {
    }

  private:
    static const  MMR920C04::BackingMap REGISTERS;
};

const MMR920C04::BackingMap MMR920C04::REGISTERS  = {
    {static_cast<uint8_t>(mmr920C04::Registers::STATUS), 0xED},
    {static_cast<uint8_t>(mmr920C04::Registers::MEASURE_MODE_1), 0},
    {static_cast<uint8_t>(mmr920C04::Registers::PRESSURE_READ), 6000},
    {static_cast<uint8_t>(mmr920C04::Registers::LOW_PASS_PRESSURE_READ),
     6000},
    {static_cast<uint8_t>(mmr920C04::Registers::TEMPERATURE_READ),
     3200}};

};  // namespace mmr920C04_simulator
