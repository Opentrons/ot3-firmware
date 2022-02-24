#include "bootloader/core/node_id.h"

#define A_CARRIER_MIN_VOLTAGE_MV (2350)
#define A_CARRIER_MAX_VOLTAGE_MV (2900)


CANNodeId determine_pipette_node_id(
    uint16_t mount_voltage_mv) {
    if ((mount_voltage_mv > A_CARRIER_MIN_VOLTAGE_MV) && (mount_voltage_mv < A_CARRIER_MAX_VOLTAGE_MV)) {
        return can_nodeid_pipette_right_bootloader;
    }
    return can_nodeid_pipette_left_bootloader;
}
