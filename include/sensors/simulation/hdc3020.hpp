#pragma once

#include "i2c/simulation/device.hpp"
#include "sensors/core/hdc3020.hpp"

namespace hdc3020_simulator {

using namespace sensors;
using namespace i2c::hardware;

class HDC3020 : public I2CRegisterMap<uint8_t, uint16_t> {
  public:
    HDC3020()
        : I2CRegisterMap(hdc3020::ADDRESS,
                         {{hdc3020::TRIGGER_ON_DEMAND_MODE, 0xAABB},
                          {hdc3020::AUTO_MEASURE_1M2S, 0xAABB},
                          {hdc3020::AUTO_MEASURE_1M1S, 0xAABB},
                          {hdc3020::AUTO_MEASURE_2M1S, 0xAABB},
                          {hdc3020::AUTO_MEASURE_4M1S, 0xAABB},
                          {hdc3020::AUTO_MEASURE_10M1S, 0xAABB},
                          {hdc3020::READ_STATUS_REGISTER, 0xAABB},
                          {hdc3020::MANUFACTURE_ID, 0}}) {}
};
};  // namespace hdc3020_simulator