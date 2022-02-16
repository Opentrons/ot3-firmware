
#include "common/simulation/i2c_sim.hpp"
#include "common/core/bit_utils.hpp"

auto sim_i2c::SimI2C::central_transmit(uint8_t *data, uint16_t size,
                                       uint16_t dev_address, uint32_t timeout)
    -> bool {
    uint8_t reg = data[0];
    uint16_t store_in_register = 0;
    auto *iter = data + 1;
    iter = bit_utils::bytes_to_int(iter, data + size, store_in_register);
    sensor_map[dev_address].REGISTER_MAP[reg] = store_in_register;
    return true;
}

auto sim_i2c::SimI2C::central_receive(uint8_t *data, uint16_t size,
                                      uint16_t dev_address, uint32_t timeout)
    -> bool {
    uint8_t reg = data[0];
    auto data_from_reg = sensor_map[dev_address].REGISTER_MAP[reg];
    auto *iter = data;
    iter = bit_utils::int_to_bytes(data_from_reg, iter, data + size);
    return true;
}
