#include "gantry/core/axis_type.h"

/**
 * Get this gantry's axis type
 *
 * @return an axis type
 */
extern "C"
GantryAxisType get_axis_type() {
    return gantry_x;
}
