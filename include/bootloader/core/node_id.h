#pragma once

#include <stdint.h>
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
 * Get the node id for a pipette from the ADC output value from reading
 * its mount interface. Implemented in core, but only relevant in
 * the pipettes.
 * */
CANNodeId determine_pipette_node_id(uint16_t sense_voltage_mv);
#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
