#pragma once

#include <stdint.h>
#include "bootloader/core/ids.h"


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Contents of the firmware update data message.
 */
typedef struct {
    uint32_t message_index;
    uint32_t address;
    uint8_t num_bytes;
    uint8_t reserved;
    const uint8_t * data;
    uint16_t checksum;
} UpdateData;

#define UPDATE_DATA_MESSAGE_SIZE    64
// can't be (UPDATE_DATA_MESSAGE_SIZE - sizeof(UpdateData) - sizeof(uint8_t))
// since c adds padding to align all struct members to 32bit addresses

// this value also needs to be a multiple of 8
#define UPDATE_DATA_MAX_BYTE_COUNT  48

/**
 * Contents of the firmware update complete message.
 */
typedef struct {
    uint32_t message_index;
    uint32_t num_messages;
    uint32_t crc32;
} UpdateComplete;

#define UPDATE_COMPLETE_MESSAGE_SIZE (sizeof(UpdateComplete))

/**
 * Get the message_index from an empty_payload message
 * @param buffer Pointer to a buffer
 * @param size The size of the buffer
 * @param result Pointer to uint32_t to populate
 * @return result code
 */
CANErrorCode parse_empty_message(
    const uint8_t * buffer,
    uint32_t size,
    uint32_t * result);


/**
 * Populate UpdateData fields from buffer.
 * @param buffer Pointer to a buffer
 * @param size The size of the buffer
 * @param result Pointer to UpdateData struct to populate
 * @return result code
 */
CANErrorCode parse_update_data(
    const uint8_t * buffer,
    uint32_t size,
    UpdateData * result);


/**
 * Populate UpdateComplete fields from buffer.
 * @param buffer Pointer to a buffer
 * @param size The size of the buffer
 * @param result Pointer to UpdateComplete struct to populate
 * @return result code
 */
CANErrorCode parse_update_complete(
    const uint8_t * buffer,
    uint32_t size,
    UpdateComplete * result);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
