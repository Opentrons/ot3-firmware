#pragma once

#include <cinttypes>

#include "common/core/bit_utils.hpp"
#include "common/core/logging.h"
#include "sensors/core/mmr920.hpp"

namespace i2c {
namespace hardware {

/**
 * Abstract base class of i2c devices.
 */
class I2CDeviceBase {
  public:
    explicit I2CDeviceBase(uint16_t address) : address{address} {}
    virtual ~I2CDeviceBase() = default;

    /** @brief Handle data transmitted to device. */
    virtual auto handle_write(const uint8_t *data, uint16_t size) -> bool = 0;

    /** @brief Handle request to read from the device. */
    virtual auto handle_read(uint8_t *data, uint16_t size) -> bool = 0;

    /** @brief Get the device address. */
    auto get_address() -> uint16_t { return address; }

  protected:
    uint16_t address;
};

/**
 * I2C device that maps registers to values.
 * @tparam RegAddressType Type of register.
 * @tparam ValueType Type of data in the register.
 */
template <typename RegAddressType, typename ValueType>
requires std::is_integral_v<ValueType> && std::is_integral_v<RegAddressType>
class I2CRegisterMap : public I2CDeviceBase {
  public:
    using BackingMap = std::map<RegAddressType, ValueType>;

    explicit I2CRegisterMap(uint16_t address, const BackingMap &reg_map)
        : I2CDeviceBase(address), register_map{reg_map} {}
    I2CRegisterMap(uint16_t address) : I2CRegisterMap(address, {}) {}

    virtual auto handle_write(const uint8_t *data, uint16_t size) -> bool {
        auto *iter = data;
        // Read the register
        iter = bit_utils::bytes_to_int(iter, data + size, current_register);

        // Is there data in the message?
        if (size > sizeof(current_register)) {
            ValueType value;
            iter = bit_utils::bytes_to_int(iter, data + size, value);

            LOG("Writing %X to register %X.", value, current_register);
            register_map[current_register] = value;

        } else {
            LOG("Update current register to %X.", current_register);
        }
        return true;
    }

    virtual auto handle_read(uint8_t *data, uint16_t size) -> bool {
        ValueType value = register_map.at(current_register);
        data = bit_utils::int_to_bytes(value, data, data + size);

        LOG("Read %+" PRIx64 " from register %X.", value, current_register);
        return true;
    }

    auto get_current_register() -> RegAddressType { return current_register; }

    auto set_current_register(RegAddressType reg) {
        current_register = reg;
        LOG("Setting current register to %X.", current_register);
    }

  private:
    BackingMap register_map;
    RegAddressType current_register{0};
};

}  // namespace hardware
}  // namespace i2c