
#include "gantry/core/axis_type.h"
#include "platform_specific_hal_conf.h"

/**
 * Get this gantry's axis type
 *
 * @return an axis type
 */
extern "C" auto get_axis_type() -> GantryAxisType { return gantry_x; }

auto get_sync_pin() -> uint8_t { return GPIO_PIN_7; }