
#include "gantry/core/axis_type.h"

/**
 * Get the node id for this gantries axis type
 *
 * @return A node id
 */
extern "C" auto get_axis_type() -> GantryAxisType { return gantry_y; }

auto get_sync_pin() -> uint8_t { return GPIO_PIN_5; }