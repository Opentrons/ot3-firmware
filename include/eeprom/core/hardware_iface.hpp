#pragma once

namespace eeprom {
namespace hardware_iface {

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
    EEPromHardwareIface(size_t addr_bytes) : eeprom_addr_bytes(addr_bytes) {}
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
    size_t get_eeprom_addr_bytes() { return eeprom_addr_bytes; }

  private:
    // The number of times that disable has been called.
    uint32_t count{0};
    // How many bytes the EEProm memory address is. default to old boards 1 byte
    // address
    size_t eeprom_addr_bytes = sizeof(uint8_t);
};

}  // namespace hardware_iface
}  // namespace eeprom
