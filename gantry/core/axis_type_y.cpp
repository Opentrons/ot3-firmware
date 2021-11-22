#include "gantry/core/axis_type.h"

/**
 * Get the node id for this gantries axis type
 *
 * @return A node id
 */
extern "C" GantryAxisType get_axis_type() { return gantry_y; }
