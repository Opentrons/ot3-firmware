#include <stdint>

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

uint8_t get_sync_pin();

/**
 * Get this gantry's axis type
 *
 * @return an axis type
 */
enum GantryAxisType get_axis_type();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
