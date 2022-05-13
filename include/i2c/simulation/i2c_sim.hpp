#pragma once

#include <map>
#include <vector>

#include "device.hpp"
#include "i2c/core/hardware_iface.hpp"

namespace i2c {
namespace hardware {

/**
 * Simulation of i2c bus.
 */
class SimI2C : public I2CBase {
  public:
    using DeviceMap = std::map<uint16_t, I2CDeviceBase &>;
    SimI2C(DeviceMap device_map) : device_map{device_map} {}
    SimI2C() : device_map(DeviceMap{}) {}
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
    DeviceMap device_map;
    std::size_t transmit_count = 0;
    std::size_t receive_count = 0;
    std::vector<uint8_t> last_transmitted{};
    std::vector<uint8_t> next_receive{};
    uint16_t last_receive_length = 0;
};
};  // namespace hardware
}  // namespace i2c
