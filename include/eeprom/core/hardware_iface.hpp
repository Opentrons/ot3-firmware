#pragma once

namespace eeprom {
namespace hardware_iface {

enum class EEPromChipType { MICROCHIP_24AA02T, ST_M24128 };

enum class EEPromAddressType {
    EEPROM_ADDR_8_BIT = sizeof(uint8_t),
    EEPROM_ADDR_16_BIT = sizeof(uint16_t)
};

enum class EEpromMemorySize { MICROCHIP_256_BYTE = 256, ST_16_KBYTE = 16384 };
/**
 * Interface to eeprom. Must be implemented in FW and Simulation
 *
 * Consecutive write requests will want to have write protection disabled. The
 * enable should happen when all the write transactions have completed.
 *
 */
class EEPromHardwareIface {
  public:
    EEPromHardwareIface() = default;
    EEPromHardwareIface(EEPromChipType chip) {
        switch (chip) {
            case EEPromChipType::MICROCHIP_24AA02T:
                eeprom_addr_bytes =
                    static_cast<size_t>(EEPromAddressType::EEPROM_ADDR_8_BIT);
                eeprom_mem_size =
                    static_cast<size_t>(EEpromMemorySize::MICROCHIP_256_BYTE);
                break;
            case EEPromChipType::ST_M24128:
                eeprom_addr_bytes =
                    static_cast<size_t>(EEPromAddressType::EEPROM_ADDR_16_BIT);
                eeprom_mem_size =
                    static_cast<size_t>(EEpromMemorySize::ST_16_KBYTE);
                break;
        }
        eeprom_chip_type = chip;
    }
    EEPromHardwareIface(const EEPromHardwareIface&) = default;
    EEPromHardwareIface(EEPromHardwareIface&&) = default;
    auto operator=(EEPromHardwareIface&&) -> EEPromHardwareIface& = default;
    auto operator=(const EEPromHardwareIface&)
        -> EEPromHardwareIface& = default;
    virtual ~EEPromHardwareIface() = default;

    /**
     * Change the write protect status.
     * @param enabled true to inhibit writing
     */
    virtual void set_write_protect(bool) = 0;

    /**
     * Disable the write protect
     */
    void disable() {
        if (count++ == 0) {
            set_write_protect(false);
        }
    }

    /**
     * Enable the write protect
     */
    void enable() {
        if (count > 0) {
            if (--count == 0) {
                set_write_protect(true);
            }
        } else {
            set_write_protect(true);
        }
    }
    [[nodiscard]] auto get_eeprom_addr_bytes() const -> size_t {
        return eeprom_addr_bytes;
    }
    [[nodiscard]] auto get_eeprom_chip_type() const -> EEPromChipType {
        return eeprom_chip_type;
    }

  private:
    // The number of times that disable has been called.
    uint32_t count{0};
    // How many bytes the EEProm memory address is. default to old boards 1 byte
    // address
    size_t eeprom_addr_bytes =
        static_cast<size_t>(EEPromAddressType::EEPROM_ADDR_8_BIT);
    size_t eeprom_mem_size =
        static_cast<size_t>(EEpromMemorySize::MICROCHIP_256_BYTE);
    EEPromChipType eeprom_chip_type = EEPromChipType::MICROCHIP_24AA02T;
};

}  // namespace hardware_iface
}  // namespace eeprom
