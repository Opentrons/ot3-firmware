#include "pipettes/core/pipette_info.hpp"

namespace pipette_info {
auto get_name() -> PipetteName { return PipetteName::P1000_MULTI; }

auto get_model() -> uint16_t { return 0; }
};  // namespace pipette_info