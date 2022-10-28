#pragma once

#include "common/firmware/gpio.hpp"
#include "pipettes/core/pipette_type.h"

namespace utility_configs {

auto led_gpio(PipetteType pipette_type) -> gpio::PinConfig;

}  // namespace utility_configs