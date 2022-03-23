#include "common/simulation/i2c_sim.hpp"

#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"

auto sim_i2c::SimI2C::central_transmit(i2c::MaxMessageBuffer& data,
                                       uint16_t dev_address, uint32_t timeout)
    -> bool {
    uint8_t reg = data[0];
    uint8_t write_bit = data[1];
    LOG("Received a transmit command for: %d and have a write bit: %d", static_cast<int>(reg), static_cast<int>(write_bit));
    if (write_bit == 1) {
        uint16_t store_in_register = 0;
        auto *iter = data.begin() + 1;
        iter = bit_utils::bytes_to_int(iter, data.end(), store_in_register);
        sensor_map[dev_address].REGISTER_MAP[reg] = store_in_register;
    }
    return true;
}

auto sim_i2c::SimI2C::central_receive(i2c::MaxMessageBuffer& data,
                                      uint16_t dev_address, uint32_t timeout)
    -> bool {
    uint8_t reg = data[0];
    auto data_from_reg = sensor_map[dev_address].REGISTER_MAP[reg];
    LOG("Received a receive command for: %d with data as: %d", static_cast<int>(reg), static_cast<int>(data_from_reg));
    auto iter = data.begin();
    iter = bit_utils::int_to_bytes(data_from_reg, iter, data.end());
    LOG("Now there is data in buffer: %d, %d, %d, %d, %d",
        static_cast<int>(data[0]), static_cast<int>(data[1]),
        static_cast<int>(data[2]), static_cast<int>(data[3]),
        static_cast<int>(data[4]));
    return true;
}

auto sim_i2c::SimI2C::wait_during_poll(uint16_t delay) -> void {}