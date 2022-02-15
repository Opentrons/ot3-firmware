#pragma once

#include "can/core/types.h"


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


/**
 * Convert a CanFilterConfig to a hal value
 * @param config can filter config enum class
 * @return hal defined constant
 */
uint32_t filter_config_to_hal(CanFilterConfig config);

/**
 * Convert a CanFilterType to a hal value
 * @param type can filter type enum class
 * @return hal defined constant
 */
uint32_t filter_type_to_hal(CanFilterType type);

/**
 * Convert a length to the hal encoded length.
 * @param length length
 * @return hal encoded length
 */
uint32_t length_to_hal(CanFDMessageLength length);

/**
 * Convert a hal encoded length to a CanFDMessageLength
 * @param length hal encoded length
 * @return CanFDMessageLength
 */
CanFDMessageLength length_from_hal(uint32_t length);


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

