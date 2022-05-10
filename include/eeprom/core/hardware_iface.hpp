#pragma once

namespace eeprom {
namespace hardware_iface {

/**
 * Interface to eeprom. Must be implemented in FW and Simulation
 */
class EEPromHardwareIface {
  public:
    EEPromHardwareIface() = default;
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
};

/**
 * Class to manages the state of the write protect pin across multiple
 * transactions.
 *
 * Consecutive write requests will want to have write protection disabled. The
 * enable should happen when all the write transactions have completed.
 *
 * This class keeps the eeprom from having to keep track of the write protection
 * state.
 */
class WriteProtector {
  public:
    /**
     * Constructor
     * @param pin
     */
    explicit WriteProtector(EEPromHardwareIface& pin) : pin{pin} {}

    /**
     * Disable the write protect
     */
    void disable() {
        if (count++ == 0) {
            pin.set_write_protect(false);
        }
    }

    /**
     * Enable the write protect
     */
    void enable() {
        if (count > 0) {
            if (--count == 0) {
                pin.set_write_protect(true);
            }
        } else {
            pin.set_write_protect(true);
        }
    }

  private:
    // THe number of times that disable has been called.
    uint32_t count{0};
    EEPromHardwareIface& pin;
};

}  // namespace hardware_iface
}  // namespace eeprom