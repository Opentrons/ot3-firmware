#include "bootloader/core/node_id.h"

/**
 * Get the node id this bootloader is installed on
 * @return Node id.
 */
NodeId get_node_id(void) {
#if defined node_id_pipette
    return can_nodeid_pipette;
#elif defined node_id_head
    return can_nodeid_head;
#elif defined node_id_gantry_x
    return can_nodeid_gantry_x;
#elif defined node_id_gantry_y
    return can_nodeid_gantry_y;
#else
    #error "No node id"
#endif
}