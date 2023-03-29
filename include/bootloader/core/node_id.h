#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "bootloader/core/ids.h"


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Get the node id this bootloader is installed on. This function is
 * not implemented in core lib. Must be implemented in executables.
 * @return Node id.
 */
CANNodeId get_node_id(void);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
