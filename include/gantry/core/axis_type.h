#include <stdint.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
/**
 * Methods that are generated for specific gantry axis.
 */

enum GantryAxisType {
    gantry_x,
    gantry_y,
};

/**
 * Get this gantry's axis type
 *
 * @return an axis type
 */
enum GantryAxisType get_axis_type();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
