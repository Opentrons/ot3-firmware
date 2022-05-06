#pragma once

#include "sensors/core/mmr920C04.hpp"
#include "sensors/simulation/sensors.hpp"

namespace mmr920C04_simulator {
using namespace sensors;
class MMR920C04 : public sensor_simulator::SensorType {
  public:
    MMR920C04() {
        ADDRESS = mmr920C04::ADDRESS;
        REGISTER_MAP = {
            {static_cast<uint8_t>(mmr920C04::Registers::STATUS), 0xED},
            {static_cast<uint8_t>(mmr920C04::Registers::MEASURE_MODE_1), 0},
            {static_cast<uint8_t>(mmr920C04::Registers::PRESSURE_READ), 6000},
            {static_cast<uint8_t>(mmr920C04::Registers::LOW_PASS_PRESSURE_READ),
             6000},
            {static_cast<uint8_t>(mmr920C04::Registers::TEMPERATURE_READ),
             3200}};
    }
};
};  // namespace mmr920C04_simulator
