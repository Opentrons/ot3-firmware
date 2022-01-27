#pragma once

#include "bootloader/core/ids.h"

/**
 * Get the node id this bootloader is installed on
 * @return Node id.
 */
NodeId get_node_id(void);