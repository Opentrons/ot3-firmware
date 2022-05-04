#pragma once


namespace eeprom {
namespace write_protect {

/**
 * Change the write protect status. Must be implemented in FW and Simulation
 * @param enabled true to inhibit writing
 */
void set(bool enabled);


/**
 * Simple class to manage disabling and enabling write protect.
 *
 * Assumes that write protect is active.
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
    uint32_t count{0};
};

}
}