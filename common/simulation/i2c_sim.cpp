#include "common/simulation/i2c_sim.hpp"

#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"

auto sim_i2c::SimI2C::central_transmit(uint8_t *data, uint16_t size,
                                       uint16_t dev_address, uint32_t timeout)
    -> bool {
    uint8_t reg = data[0];
    uint8_t write_bit = data[1];
    LOG("Received a register bit: %d", reg);

    if (write_bit == 1) {
        LOG("Received a write bit");
        uint16_t store_in_register = 0;
        auto *iter = data + 1;
        iter = bit_utils::bytes_to_int(iter, data + size, store_in_register);
        LOG("Storing %d in register %d", store_in_register, reg);
        sensor_map[dev_address].REGISTER_MAP[reg] = store_in_register;
    }
    return true;
}

auto sim_i2c::SimI2C::central_receive(uint8_t *data, uint16_t size,
                                      uint16_t dev_address, uint32_t timeout)
    -> bool {
    uint8_t reg = data[0];
    auto data_from_reg = sensor_map[dev_address].REGISTER_MAP[reg];
    LOG("Grabbing data %d from register %d", data_from_reg, reg);
    auto *iter = data;
    iter = bit_utils::int_to_bytes(data_from_reg, iter, data + size);
    return true;
}

auto sim_i2c::SimI2C::wait_during_poll(uint16_t delay) -> void {}