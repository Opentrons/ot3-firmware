#pragma once

#include <array>

#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/types.hpp"
#include "i2c/simulation/device.hpp"

namespace eeprom {
namespace simulator {

using namespace i2c::hardware;

class EEProm : public I2CDeviceBase,
               public hardware_iface::EEPromHardwareIface {
  public:
    EEProm() : I2CDeviceBase(types::DEVICE_ADDRESS) { backing.fill(0xFF); }
    EEProm(hardware_iface::EEPromChipType chip)
        : I2CDeviceBase(types::DEVICE_ADDRESS),
          hardware_iface::EEPromHardwareIface(chip) {
        backing.fill(0xFF);
    }

    auto handle_write(const uint8_t *data, uint16_t size) -> bool {
        auto *iter = data;

        // Read the address
        iter = bit_utils::bytes_to_int(
            iter, data + this->get_eeprom_addr_bytes(), current_address);
        // if the address is configued as 8 bit we need to shift the
        // current_address toward the LSB
        if (this->get_eeprom_addr_bytes() ==
            static_cast<size_t>(
                hardware_iface::EEPromAddressType::EEPROM_ADDR_8_BIT)) {
            current_address = current_address >> 8;
        }
        auto data_size = size - this->get_eeprom_addr_bytes();
        if (data_size > 0) {
            if (!write_protected) {
                // Let the exception happen. Catch errors!
                std::copy_n(iter, data_size, backing.begin() + current_address);
                LOG("Writing %d bytes to address %X", data_size,
                    current_address);
            } else {
                LOG("Write protect is enabled. Cannot write.");
            }
        } else {
            LOG("Updating address to %X", current_address);
        }

        return true;
    }

    auto handle_read(uint8_t *data, uint16_t size) -> bool {
        LOG("Reading %d bytes from address %X", size, current_address);
        for (auto i = 0; i < size; i++) {
            data[i] = backing[current_address++];
        }
        return true;
    }

    void set_write_protect(bool enabled) final {
        LOG("Setting write protect enabled to '%s'",
            enabled ? "enabled" : "disabled");
        write_protected = enabled;
    }

  private:
    std::array<uint8_t, static_cast<size_t>(
                            hardware_iface::EEpromMemorySize::ST_16_KBYTE)>
        backing{};
    types::address current_address{0};
    bool write_protected{true};
};

}  // namespace simulator
}  // namespace eeprom
