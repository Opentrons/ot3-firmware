#pragma once

#include "sensors/core/fdc1004.hpp"
#include "sensors/simulation/sensors.hpp"

namespace fdc1004_simulator {
class FDC1004 : public sensor_simulator::SensorType {
  public:
    FDC1004() {
        DEVICE_ID = fdc1004_utils::DEVICE_ID;
        ADDRESS = fdc1004_utils::ADDRESS;
        REGISTER_MAP = {{fdc1004_utils::CONFIGURATION_MEASUREMENT, 0},
                        {fdc1004_utils::FDC_CONFIGURATION, 0},
                        {fdc1004_utils::MEASURE_REGISTER, 0},
                        {fdc1004_utils::MSB_MEASUREMENT_1, 5},
                        {fdc1004_utils::LSB_MEASUREMENT_1, 2},
                        {fdc1004_utils::DEVICE_ID_REGISTER, fdc1004_utils::DEVICE_ID}};
    }
};
};  // namespace fdc1004_simulator