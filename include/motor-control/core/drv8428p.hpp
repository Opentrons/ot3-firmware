#pragme once

#include "drv8428p_config.hpp"

namespace drv8428p {

constexpr float VOLTAGE_REFERENCE = 3.3;
constexpr int DAC_DATA_LENGTH = 12;
constexpr int DAC_DATA_MULTIPLIER = 2 ^ DAC_DATA_LENGTH - 1;

class DRV8428P {
  public:
    DRV8428P() = delete;
    DRV8428P() : _initialized(false) {}

    void set_vref(float val) {
        auto vref = static_cast<uint32_t>(val * DAC_DATA_MULTIPLIER /
                                          VOLTAGE_REFERENCE);
        return vref;
    }
};
}  // namespace drv8428p