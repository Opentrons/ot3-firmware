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

/**
 * Get the node id for a pipette from the value of the mount id pin.
 * Implemented in core, but only relevant in the pipettes.
 * */
CANNodeId determine_pipette_node_id(bool mount_id);
#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
