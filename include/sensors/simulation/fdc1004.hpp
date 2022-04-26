#pragma once

#include "sensors/core/fdc1004.hpp"
#include "sensors/simulation/sensors.hpp"

namespace fdc1004_simulator {
using namespace sensors;
class FDC1004 : public sensor_simulator::SensorType {
  public:
    FDC1004() {
        DEVICE_ID = fdc1004::DEVICE_ID;
        ADDRESS = fdc1004::ADDRESS;
        REGISTER_MAP = {{fdc1004::CONFIGURATION_MEASUREMENT, 0},
                        {fdc1004::FDC_CONFIGURATION, 0},
                        {fdc1004::MSB_MEASUREMENT_1, 1},
                        {fdc1004::LSB_MEASUREMENT_1, 200},
                        {fdc1004::DEVICE_ID_REGISTER, fdc1004::DEVICE_ID}};
    }
};
};  // namespace fdc1004_simulator
