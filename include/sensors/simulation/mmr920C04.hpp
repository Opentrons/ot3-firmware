#pragma once

#include "i2c/simulation/device.hpp"
#include "sensors/core/mmr920.hpp"
#include "sensors/simulation/mock_hardware.hpp"

namespace mmr920_simulator {

using namespace sensors;
using namespace i2c::hardware;

class MMR920 : public I2CRegisterMap<uint8_t, uint32_t> {
  public:
    MMR920()
        : I2CRegisterMap(
              mmr920::ADDRESS,
              {{static_cast<uint8_t>(mmr920::Registers::STATUS), 0xED},
               {static_cast<uint8_t>(mmr920::Registers::MEASURE_MODE_1), 0},
               {static_cast<uint8_t>(mmr920::Registers::PRESSURE_READ), 6000},
               {static_cast<uint8_t>(mmr920::Registers::LOW_PASS_PRESSURE_READ),
                6000},
               {static_cast<uint8_t>(mmr920::Registers::TEMPERATURE_READ),
                3200}}) {}
};

};  // namespace mmr920_simulator
