#pragma once

#include <stdint.h>
#include "bootloader/core/update_state.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * The application firmware updater.
 * These functions will be implemented in simulation and in firmware.
 */

typedef enum {
    fw_update_ok,
    fw_update_error,
    fw_update_invalid_data,
    fw_update_invalid_size
} FwUpdateReturn;

/**
 * Initialize fw update state.
 * @param state the update state
 * @return Result
 */
FwUpdateReturn fw_update_initialize(UpdateState* state);

/**
 * Handle a chunk of firmware update data
 * @param state the update state
 * @param address where to write the data
 * @param data pointer to the buffer
 * @param length how long the buffer is in bytes
 * @return Result
 */
FwUpdateReturn fw_update_data(UpdateState* state, uint32_t address, const uint8_t* data, uint8_t length);

/**
 * Complete the update.
 * @param state the update state
 * @param num_messages how many messages were part of the update.
 * @param error_detection crc32/checksum of the data contents of the update.
 * @return Result
 */
FwUpdateReturn fw_update_complete(UpdateState* state, uint32_t num_messages, uint32_t error_detection);

/**
 * Start the application. This function will not return.
 */
void fw_update_start_application(void);


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
