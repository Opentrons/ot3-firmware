#pragma once

#include <stdint.h>
#include "bootloader/core/ids.h"


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Contents of the firmware update data message.
 */
struct UpdateData {
    uint32_t address;
    uint8_t num_bytes;
    uint8_t reserved;
    const uint8_t * data;
    uint16_t checksum;
};

#define UPDATE_DATA_MESSAGE_SIZE    64
#define UPDATE_DATA_MAX_BYTE_COUNT  56

/**
 * Contents of the firmware update complete message.
 */
struct UpdateComplete {
    uint32_t num_messages;
};

#define UPDATE_COMPLETE_MESSAGE_SIZE    4


/**
 * Populate UpdateData fields from buffer. 
 * @param buffer Pointer to a buffer
 * @param size The size of the buffer
 * @param result Pointer to UpdateData struct to populate
 * @return result code
 */
enum ErrorCode parse_update_data(
    const uint8_t * buffer,
    uint32_t size,
    struct UpdateData * result);


/**
 * Populate UpdateComplete fields from buffer.
 * @param buffer Pointer to a buffer
 * @param size The size of the buffer
 * @param result Pointer to UpdateComplete struct to populate
 * @return result code
 */
enum ErrorCode parse_update_complete(
    const uint8_t * buffer,
    uint32_t size,
    struct UpdateData * result);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
