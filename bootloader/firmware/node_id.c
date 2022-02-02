#include "bootloader/core/node_id.h"

/**
 * Get the node id this bootloader is installed on
 * @return Node id.
 */
CANNodeId get_node_id(void) {
#if defined node_id_pipette_left
    // TODO make this decision based on the mount sense voltage
    return can_nodeid_pipette_left_bootloader;
#elif defined node_id_head
    return can_nodeid_head_bootloader;
#elif defined node_id_gantry_x
    return can_nodeid_gantry_x_bootloader;
#elif defined node_id_gantry_y
    return can_nodeid_gantry_y_bootloader;
#else
    #error "No node id"
#endif
}
