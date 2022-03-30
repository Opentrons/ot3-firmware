#pragma once

#include "sensors/core/mmr920C04_registers.hpp"
#include "sensors/simulation/sensors.hpp"

namespace mmr920C04_simulator {
class MMR920C04 : public sensor_simulator::SensorType {
  public:
    MMR920C04() {
        ADDRESS = mmr920C04_registers::ADDRESS;
        REGISTER_MAP = {
            {static_cast<uint8_t>(mmr920C04_registers::Registers::STATUS),
             0xED},
            {static_cast<uint8_t>(
                 mmr920C04_registers::Registers::MEASURE_MODE_1),
             0},
            {static_cast<uint8_t>(
                 mmr920C04_registers::Registers::PRESSURE_READ),
             3000000},
            {static_cast<uint8_t>(
                 mmr920C04_registers::Registers::LOW_PASS_PRESSURE_READ),
             3000000},
            {static_cast<uint8_t>(
                 mmr920C04_registers::Registers::TEMPERATURE_READ),
             3200}};
    }
};
};  // namespace mmr920C04_simulator