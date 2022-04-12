#include "i2c/simulation/i2c_sim.hpp"

#include <map>
#include <vector>

#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
using namespace i2c::hardware;
auto SimI2C::central_transmit(uint8_t *data, uint16_t size,
                              uint16_t dev_address, uint32_t timeout) -> bool {
    uint8_t reg = data[0];
    uint8_t write_bit = data[1];
    LOG("Received a register bit: %d", reg);
    last_transmitted = std::vector<uint8_t>(size);
    std::copy_n(data, size, last_transmitted.begin());
    transmit_count++;
    if (!sensor_map.empty()) {
        uint16_t store_in_register = 0;
        auto *iter = data + 1;
        iter = bit_utils::bytes_to_int(iter, data + size, store_in_register);
        LOG("Storing %d in register %d", store_in_register, reg);
        sensor_map[dev_address].REGISTER_MAP[reg] = store_in_register;
        next_register_map[dev_address] = reg;
    }

    return true;
}

auto SimI2C::central_receive(uint8_t *data, uint16_t size, uint16_t dev_address,
                             uint32_t timeout) -> bool {
    auto next_reg = next_register_map[dev_address];
    // This will raise if the register value is bad - helpful for forcing
    // tests to fail
    if (!sensor_map.empty()) {
        auto data_from_reg = sensor_map[dev_address].REGISTER_MAP.at(next_reg);
        auto *iter = data;
        iter = bit_utils::int_to_bytes(data_from_reg, iter, data + size);
        next_register_map[dev_address] = 0;
    } else {
        std::copy_n(
            next_receive.cbegin(),
            std::min(static_cast<decltype(next_receive)::size_type>(size),
                     next_receive.size()),
            data);
    }
    receive_count++;
    last_receive_length = size;
    return true;
}

auto SimI2C::build_next_register_map(const SensorMap &sm) -> NextRegisterMap {
    NextRegisterMap to_ret{};
    for (auto &[addr, sensor] : sm) {
        to_ret.insert({addr, sensor.REGISTER_MAP.cbegin()->first});
    }
    return to_ret;
}

auto SimI2C::get_last_transmitted() const -> const std::vector<uint8_t> & {
    return last_transmitted;
}

auto SimI2C::get_transmit_count() const -> std::size_t {
    return transmit_count;
}

auto SimI2C::get_receive_count() const -> std::size_t { return receive_count; }

auto SimI2C::set_next_received(const std::vector<uint8_t> &to_receive) -> void {
    next_receive = to_receive;
}

auto SimI2C::get_last_receive_length() const -> std::size_t {
    return last_receive_length;
}
