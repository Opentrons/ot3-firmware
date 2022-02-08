#pragma once

#include <stdint.h>

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
 */
FwUpdateReturn fw_update_initialize(void);

/**
 * Handle a chunk of firmware update data
 * @param address where to write the data
 * @param data pointer to the buffer
 * @param length how long the buffer is in bytes
 * @return Result
 */
FwUpdateReturn fw_update_data(uint32_t address, const uint8_t* data, uint8_t length);

/**
 * Complete the update.
 * @param num_messages how many messages were part of the update.
 * @param error_detection crc32/checksum of the data contents of the update.
 * @return Result
 */
FwUpdateReturn fw_update_complete(uint32_t num_messages, uint32_t error_detection);


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
