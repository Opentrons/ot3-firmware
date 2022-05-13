#pragma once

#include "i2c/simulation/device.hpp"
#include "eeprom/core/types.hpp"
#include "common/core/logging.h"
#include <array>
#include "common/core/bit_utils.hpp"


namespace eeprom {
namespace simulator {

using namespace i2c::hardware;

class EEProm : public I2CDeviceBase {
  public:
    EEProm() : I2CDeviceBase(types::DEVICE_ADDRESS) {
        backing.fill(0xFF);
    }

    auto handle_write(const uint8_t *data , uint16_t size) -> bool {
        auto* iter = data;
        // Read the address
        iter = bit_utils::bytes_to_int(iter, data + size, current_address);
        auto data_size =size - sizeof(current_address);

        if (data_size > 0) {
            // Let the exception happen. Catch errors!
            std::copy_n(iter, data_size, backing.begin() + current_address);
            LOG("Writing %d bytes to address %X", data_size, current_address);
        } else {
            LOG("Updating address to %X", current_address);
        }

        return true;
    }

    auto handle_read(uint8_t *data , uint16_t size) -> bool  {
        LOG("Reading %d bytes from address %X", size, current_address);
        for (auto i = 0; i < size; i++) {
            data[i] = backing[current_address++];
        }
        return true;
    }
  private:
    std::array<uint8_t, 256> backing{};
    uint8_t current_address{0};

};

}  // namespace simulator
}  // namespace eeprom