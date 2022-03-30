
#include "gantry/core/axis_type.h"

/**
 * Get this gantry's axis type
 *
 * @return an axis type
 */
extern "C" auto get_axis_type() -> GantryAxisType { return gantry_x; }