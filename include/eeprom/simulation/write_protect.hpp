#pragma once

#include "eeprom/core/write_protect.hpp"
#include "common/core/logging.h"

namespace eeprom {
namespace sim_write_protect {

class SimWriteProtectPin : public eeprom::write_protect::WriteProtectPin {

  public:
    void set(bool enabled) final {
        LOG("eeprom write protect enabled=%d", enabled);
    }
};


}}