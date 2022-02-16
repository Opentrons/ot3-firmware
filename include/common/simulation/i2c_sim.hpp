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
    SimI2C(SensorMap sensor_map) : sensor_map{sensor_map} {}
    auto central_transmit(uint8_t *data, uint16_t size, uint16_t dev_address,
                          uint32_t timeout) -> bool final;
    auto central_receive(uint8_t *data, uint16_t size, uint16_t dev_address,
                         uint32_t timeout) -> bool final;

  private:
    SensorMap sensor_map;
};

}  // namespace sim_i2c