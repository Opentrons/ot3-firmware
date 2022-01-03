
#include "common/simulation/eeprom.hpp"

auto sim_i2c::SimEEProm::master_transmit(uint16_t dev_address, uint8_t *data,
                                         uint16_t size, uint32_t timeout)
    -> bool {
    this->data.clear();
    this->data.reserve(size);
    std::copy(data, data + size, std::back_inserter(this->data));
    return true;
}

auto sim_i2c::SimEEProm::master_receive(uint16_t dev_address, uint8_t *data,
                                        uint16_t size, uint32_t timeout)
    -> bool {
    std::copy(this->data.cbegin(), this->data.cend(), data);
    return true;
}
