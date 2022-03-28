#pragma once

#include "sensors/core/mmr920C04_registers.hpp"
#include "sensors/simulation/sensors.hpp"

namespace mmr920C04_simulator {
class MMR920C04 : public sensor_simulator::SensorType {
  public:
    MMR920C04() {
        DEVICE_ID = mmr920C04_utils::DEVICE_ID;
        ADDRESS = mmr920C04_utils::ADDRESS;
        REGISTER_MAP = {
            {mmr920C04_utils::CONFIGURATION_MEASUREMENT, 0},
            {mmr920C04_utils::FDC_CONFIGURATION, 0},
            {mmr920C04_utils::MSB_MEASUREMENT_1, 5},
            {mmr920C04_utils::LSB_MEASUREMENT_1, 2},
            {mmr920C04_utils::DEVICE_ID_REGISTER, mmr920C04_utils::DEVICE_ID}};
    }
};
};  // namespace mmr920C04_simulator