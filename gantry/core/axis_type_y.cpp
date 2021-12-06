#include "gantry/core/axis_type.h"

/**
 * Get the node id for this gantries axis type
 *
 * @return A node id
 */
extern "C" auto get_axis_type() -> GantryAxisType { return gantry_y; }
