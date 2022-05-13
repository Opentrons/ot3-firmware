#pragma once

#include "sensors/core/hdc2080.hpp"
#include "i2c/simulation/device.hpp"

namespace hdc2080_simulator {

using namespace sensors;
using namespace i2c::hardware;

class HDC2080 : public I2CRegisterMap<uint8_t, uint16_t> {
  public:
    HDC2080(): I2CRegisterMap(hdc2080::ADDRESS, {{hdc2080::INTERRUPT_REGISTER, 0},
                        {hdc2080::DRDY_CONFIG, 0},
                        {hdc2080::MEASURE_REGISTER, 0},
                        {hdc2080::LSB_TEMPERATURE_REGISTER, 25000},
                        {hdc2080::LSB_HUMIDITY_REGISTER, 55000},
                        {hdc2080::MSB_TEMPERATURE_REGISTER, 50000},
                        {hdc2080::MSB_HUMIDITY_REGISTER, 40000},
                        {hdc2080::DEVICE_ID_REGISTER, 0}}) {
    }
};
};  // namespace hdc2080_simulator
