#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "pipettes/core/pipette_type.h"

namespace interfaces {
auto driver_config_by_axis(PipetteType which)
    -> tmc2130::configs::TMC2130DriverConfig;
}