#include "pipettes/core/pipette_info.hpp"
//#include "pipettes/core/pipette_type.h"

//extern "C" const auto get_pipette_type() -> PipetteType {
//    return THREE_EIGHTY_FOUR_CHANNEL;
//}

namespace pipette_info {
auto get_name() -> PipetteName { return PipetteName::P1000_384; }

auto get_model() -> uint16_t { return 0; }
};  // namespace pipette_info
