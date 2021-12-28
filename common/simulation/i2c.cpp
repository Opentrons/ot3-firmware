
#include "common/simulation/i2c.hpp"



auto i2c::SimI2C::master_transmit(uint16_t dev_address, uint8_t *data, uint16_t size,
                         uint32_t timeout) -> bool  {
    this->data.clear();
    for (auto i = 0; i < size; i++) {
        this->data.push_back(data[i]);
    }
    return true;
}

auto i2c::SimI2C::master_receive(uint16_t dev_address, uint8_t *data, uint16_t size,
                        uint32_t timeout) -> bool  {
    for (uint32_t i = 0; i < size && i < this->data.size(); i++) {
        data[i] = this->data[i];
    }
    return true;
}

