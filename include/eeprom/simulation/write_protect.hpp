#pragma once

#include "common/core/logging.h"
#include "eeprom/core/write_protect.hpp"

namespace eeprom {
namespace sim_write_protect {

class SimWriteProtectPin : public eeprom::write_protect::WriteProtectPin {
  public:
    void set(bool enabled) final {
        LOG("eeprom write protect enabled=%d", enabled);
    }
};

}  // namespace sim_write_protect
}  // namespace eeprom