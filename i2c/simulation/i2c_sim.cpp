#include "i2c/simulation/i2c_sim.hpp"

#include <map>
#include <vector>

#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"

using namespace i2c::hardware;
auto SimI2C::central_transmit(uint8_t *data, uint16_t size,
                              uint16_t dev_address, uint32_t) -> bool {
    last_transmitted = std::vector<uint8_t>(size);
    std::copy_n(data, size, last_transmitted.begin());
    transmit_count++;

    auto &i2c_device = device_map.at(dev_address);
    return i2c_device.handle_write(data, size);
}

auto SimI2C::central_receive(uint8_t *data, uint16_t size, uint16_t dev_address,
                             uint32_t) -> bool {
    auto ret_val = true;
    if (!device_map.empty()) {
        auto &i2c_device = device_map.at(dev_address);
        ret_val = i2c_device.handle_read(data, size);
    } else {
        std::copy_n(
            next_receive.cbegin(),
            std::min(static_cast<decltype(next_receive)::size_type>(size),
                     next_receive.size()),
            data);
    }
    receive_count++;
    last_receive_length = size;
    return ret_val;
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
