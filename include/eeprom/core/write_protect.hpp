#pragma once

namespace eeprom {
namespace write_protect {

/**
 * Interface to write protect pin. Must be implemented in FW and Simulation
 */
class WriteProtectPin {
  public:
    WriteProtectPin() = default;
    WriteProtectPin(const WriteProtectPin&) = default;
    WriteProtectPin(WriteProtectPin&&) = default;
    auto operator=(WriteProtectPin&&) -> WriteProtectPin& = default;
    auto operator=(const WriteProtectPin&) -> WriteProtectPin& = default;
    virtual ~WriteProtectPin() = default;

    /**
     * Change the write protect status.
     * @param enabled true to inhibit writing
     */
    virtual void set(bool) = 0;
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
    explicit WriteProtector(WriteProtectPin& pin) : pin{pin} {}

    /**
     * Disable the write protect
     */
    void disable() {
        if (count++ == 0) {
            pin.set(false);
        }
    }

    /**
     * Enable the write protect
     */
    void enable() {
        if (count > 0) {
            if (--count == 0) {
                pin.set(true);
            }
        } else {
            pin.set(true);
        }
    }

  private:
    // THe number of times that disable has been called.
    uint32_t count{0};
    WriteProtectPin& pin;
};

}  // namespace write_protect
}  // namespace eeprom