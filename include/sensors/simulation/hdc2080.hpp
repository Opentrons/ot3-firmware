#pragma once

#include "sensors/core/hdc2080.hpp"
#include "sensors/simulation/sensors.hpp"

namespace hdc2080_simulator {
class HDC2080 : public sensor_simulator::SensorType {
  public:
    HDC2080() {
        DEVICE_ID = hdc2080_utils::DEVICE_ID;
        ADDRESS = hdc2080_utils::ADDRESS;
        REGISTER_MAP = {{hdc2080_utils::INTERRUPT_REGISTER, 0},
                        {hdc2080_utils::DRDY_CONFIG, 0},
                        {hdc2080_utils::MEASURE_REGISTER, 0},
                        {hdc2080_utils::LSB_TEMPERATURE_REGISTER, 25000},
                        {hdc2080_utils::LSB_HUMIDITY_REGISTER, 55000},
                        {hdc2080_utils::MSB_TEMPERATURE_REGISTER, 50000},
                        {hdc2080_utils::MSB_HUMIDITY_REGISTER, 40000}};
    }
};
};  // namespace hdc2080_simulator