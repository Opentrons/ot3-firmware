#pragma once

namespace eeprom {
namespace hardware_iface {

enum EEPromAddressType {
	EEPROM_ADDR_8_BIT = sizeof(uint8_t),
	EEPROM_ADDR_16_BIT = sizeof(uint16_t)
};
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
    EEPromHardwareIface(const size_t addr_bytes)
        : eeprom_addr_bytes(addr_bytes) {}
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

  private:
    // The number of times that disable has been called.
    uint32_t count{0};
    // How many bytes the EEProm memory address is. default to old boards 1 byte
    // address
    size_t eeprom_addr_bytes = EEPROM_ADDR_8_BIT;
};

}  // namespace hardware_iface
}  // namespace eeprom
