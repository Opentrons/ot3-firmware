#pragma once

#include "sensors/core/hdc2080.hpp"
#include "sensors/simulation/sensors.hpp"

namespace hdc2080_simulator {
using namespace sensors;
class HDC2080 : public sensor_simulator::SensorType {
  public:
    HDC2080() {
        DEVICE_ID = hdc2080::DEVICE_ID;
        ADDRESS = hdc2080::ADDRESS;
        REGISTER_MAP = {{hdc2080::INTERRUPT_REGISTER, 0},
                        {hdc2080::DRDY_CONFIG, 0},
                        {hdc2080::MEASURE_REGISTER, 0},
                        {hdc2080::LSB_TEMPERATURE_REGISTER, 25000},
                        {hdc2080::LSB_HUMIDITY_REGISTER, 55000},
                        {hdc2080::MSB_TEMPERATURE_REGISTER, 50000},
                        {hdc2080::MSB_HUMIDITY_REGISTER, 40000}};
    }
};
};  // namespace hdc2080_simulator
