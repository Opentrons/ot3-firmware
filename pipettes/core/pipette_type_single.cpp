#include "pipettes/core/pipette_type.h"

extern "C" auto get_pipette_type() -> PipetteType { return SINGLE_CHANNEL; }