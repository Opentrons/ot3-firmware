#pragma once

#include "common/core/logging.h"
#include "eeprom/core/hardware_iface.hpp"

namespace eeprom {
namespace sim_hardware_iface {

class SimEEPromHardwareIface
    : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    void set_write_protect(bool enabled) final {
        LOG("eeprom write protect enabled=%d", enabled);
    }
};

}  // namespace sim_hardware_iface
}  // namespace eeprom