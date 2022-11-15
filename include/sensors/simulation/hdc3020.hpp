#pragma once

#include "i2c/simulation/device.hpp"
#include "sensors/core/hdc3020.hpp"

namespace hdc3020_simulator {

using namespace sensors;
using namespace i2c::hardware;

class HDC3020 : public I2CRegisterMap<uint8_t, uint64_t> {
  public:
    HDC3020()
        : I2CRegisterMap(
              hdc3020::ADDRESS,
              {{static_cast<uint8_t>(
                    hdc3020::Registers::TRIGGER_ON_DEMAND_MODE),
                0x5090312096310000},
               {static_cast<uint8_t>(hdc3020::Registers::AUTO_MEASURE_1M2S),
                0x00},
               {static_cast<uint8_t>(hdc3020::Registers::AUTO_MEASURE_1M1S),
                0x00},
               {static_cast<uint8_t>(hdc3020::Registers::AUTO_MEASURE_2M1S),
                0x00},
               {static_cast<uint8_t>(hdc3020::Registers::AUTO_MEASURE_4M1S),
                0x00},
               {static_cast<uint8_t>(hdc3020::Registers::AUTO_MEASURE_10M1S),
                0x00},
               {static_cast<uint8_t>(hdc3020::Registers::AUTO_MEASURE_STATUS),
                0x5090312096310000}}) {}

    auto handle_write(const uint8_t *data, uint16_t size) -> bool {
        auto *iter = data;
        // Read the register
        uint8_t temp_current_register = 0x0;
        iter =
            bit_utils::bytes_to_int(iter, data + size, temp_current_register);
        I2CRegisterMap<uint8_t, uint64_t>::set_current_register(
            temp_current_register);
        return true;
    }
};
};  // namespace hdc3020_simulator