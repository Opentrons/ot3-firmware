
#include "common/simulation/i2c_sim.hpp"

auto sim_i2c::SimI2C::master_transmit(uint8_t *data, uint16_t size,
                                      uint16_t dev_address, uint32_t timeout)
    -> bool {
    this->storage.clear();
    this->storage.reserve(size);
    std::copy(data, data + size, std::back_inserter(this->storage));
    return true;
}

auto sim_i2c::SimI2C::master_receive(uint8_t *data, uint16_t size,
                                     uint16_t dev_address, uint32_t timeout)
    -> bool {
    std::copy(this->storage.cbegin(), this->storage.cend(), data);
    return true;
}