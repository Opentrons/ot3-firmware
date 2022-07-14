#pragma once

#include "i2c/simulation/device.hpp"
#include "sensors/core/mmr920C04.hpp"

namespace mmr920C04_simulator {

using namespace sensors;
using namespace i2c::hardware;

class MMR920C04 : public I2CRegisterMap<uint8_t, uint32_t> {
  public:
    MMR920C04(test_mocks::MockSensorHardware &mock_sensor_hw)
        : I2CRegisterMap{
              mmr920C04::ADDRESS,
              {{static_cast<uint8_t>(mmr920C04::Registers::STATUS), 0xED},
               {static_cast<uint8_t>(mmr920C04::Registers::MEASURE_MODE_1), 0},
               {static_cast<uint8_t>(mmr920C04::Registers::PRESSURE_READ),
                6000},
               {static_cast<uint8_t>(
                    mmr920C04::Registers::LOW_PASS_PRESSURE_READ),
                6000},
               {static_cast<uint8_t>(mmr920C04::Registers::TEMPERATURE_READ),
                3200}}}, mock_sensor_hardware{mock_sensor_hw} {}

        auto handle_write(const uint8_t *data, uint16_t size) -> bool {
            auto result = I2CRegisterMap<uint8_t, uint32_t>::handle_write(data, size);
            uint8_t curr_reg = static_cast<uint8_t>(I2CRegisterMap<uint8_t, uint32_t>::get_current_register());
            if (curr_reg == static_cast<uint8_t>(mmr920C04::Registers::MEASURE_MODE_4)) {
                mock_sensor_hardware.data_ready();
            }
            return result;
        }

    test_mocks::MockSensorHardware& mock_sensor_hardware;
};

};  // namespace mmr920C04_simulator
