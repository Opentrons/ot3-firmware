#include "bootloader/core/node_id.h"

CANNodeId determine_pipette_node_id(
    bool mount_id) {
    return mount_id ? can_nodeid_pipette_left_bootloader : can_nodeid_pipette_right_bootloader;
}
