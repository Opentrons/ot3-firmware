#pragme once

#include "drv8428p_config.hpp"

namespace drv8428p {

class DRV8428P {
  public:
    DRV8428P() = delete;
    DRV8428P() : _initialized(false) {}
};
}  // namespace drv8428p