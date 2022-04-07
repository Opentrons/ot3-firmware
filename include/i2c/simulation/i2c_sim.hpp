#pragma once

#include <map>
#include <vector>

#include "i2c/core/hardware_iface.hpp"
#include "sensors/simulation/sensors.hpp"

namespace i2c {
namespace hardware {

/**
 * Simulation of eeprom on i2c. Read back whatever was written.
 */
class SimI2C : public I2CDeviceBase {
  public:
    using SensorMap = std::map<uint16_t, sensor_simulator::SensorType>;
    SimI2C(SensorMap sensor_map)
        : sensor_map{sensor_map},
          next_register_map(build_next_register_map(sensor_map)) {}
    SimI2C()
        : sensor_map(SensorMap{}),
          next_register_map(std::map<uint16_t, uint8_t>{}) {}
    auto central_transmit(uint8_t *data, uint16_t size, uint16_t dev_address,
                          uint32_t timeout) -> bool final;
    auto central_receive(uint8_t *data, uint16_t size, uint16_t dev_address,
                         uint32_t timeout) -> bool final;
    auto get_transmit_count() const -> std::size_t;
    auto get_receive_count() const -> std::size_t;
    auto get_last_receive_length() const -> std::size_t;
    auto get_last_transmitted() const -> const std::vector<uint8_t> &;
    auto set_next_received(const std::vector<uint8_t> &to_receive) -> void;

  private:
    using NextRegisterMap = std::map<uint16_t, uint8_t>;
    static auto build_next_register_map(const SensorMap &sm) -> NextRegisterMap;
    SensorMap sensor_map;
    std::map<uint16_t, uint8_t> next_register_map;
    std::size_t transmit_count = 0;
    std::size_t receive_count = 0;
    std::vector<uint8_t> last_transmitted{};
    std::vector<uint8_t> next_receive{};
    uint16_t last_receive_length = 0;
};
};  // namespace hardware
}  // namespace i2c
