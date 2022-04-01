#pragma once

#include <map>
#include <vector>

#include "common/core/i2c.hpp"
#include "sensors/simulation/sensors.hpp"

namespace sim_i2c {

/**
 * Simulation of eeprom on i2c. Read back whatever was written.
 */
class SimI2C : public i2c::I2CDeviceBase {
  public:
    using SensorMap = std::map<uint16_t, sensor_simulator::SensorType>;
    SimI2C(SensorMap sensor_map)
        : sensor_map{sensor_map},
          next_register_map(build_next_register_map(sensor_map)) {}
    auto central_transmit(uint8_t *data, uint16_t size, uint16_t dev_address,
                          uint32_t timeout) -> bool final;
    auto central_receive(uint8_t *data, uint16_t size, uint16_t dev_address,
                         uint32_t timeout) -> bool final;
    auto wait_during_poll(uint16_t delay) -> void final;

  private:
    using NextRegisterMap = std::map<uint16_t, uint8_t>;
    static auto build_next_register_map(const SensorMap &sm) -> NextRegisterMap;
    SensorMap sensor_map;
    std::map<uint16_t, uint8_t> next_register_map;
};

}  // namespace sim_i2c
