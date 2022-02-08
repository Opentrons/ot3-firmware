#pragma once

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Get the bootloader version. This function is not implemented in core
 * lib. Must be implemented in executables.
 * @return version.
 */
uint32_t get_version(void);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
