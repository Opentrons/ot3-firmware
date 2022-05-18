#include "pipettes/core/pipette_info.hpp"
//#include "pipettes/core/pipette_type.h"

// extern "C" const auto get_pipette_type() -> PipetteType { return
// SINGLE_CHANNEL; }

namespace pipette_info {
auto get_name() -> PipetteName { return PipetteName::P1000_SINGLE; }

auto get_model() -> uint16_t { return 0; }
};  // namespace pipette_info
