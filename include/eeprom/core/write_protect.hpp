#pragma once


namespace eeprom {
namespace write_protect {

/**
 * Change the write protect status. Must be implemented in FW and Simulation
 * @param enabled true to inhibit writing
 */
void set(bool enabled);


/**
 * Class to manages the state of the write protect pin across multiple transactions.
 *
 * Consecutive write requests will want to have write protection disabled. The enable should
 * happen when all the write transactions have completed.
 *
 * This class keeps the eeprom from having to keep track of the write protection state.
 */
class WriteProtector {
  public:
    /**
     * Disable the write protect
     */
    void disable() {
        if (count++ == 0) {
            set(false);
        }
    }

    /**
     * Enable the write protect
     */
    void enable() {
        if (count > 0) {
            if (--count == 0) {
                set(true);
            }
        } else {
            set(true);
        }
    }
  private:
    // THe number of times that disable has been called.
    uint32_t count{0};
};

}
}