#include "bootloader/core/messages.h"
#include "bootloader/core/util.h"

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
    UpdateData * result) {

    if (!result) {
        return can_errorcode_invalid_input;
    }

    // Message size and null check
    if (!buffer || size != UPDATE_DATA_MESSAGE_SIZE) {
        return can_errorcode_invalid_size;
    }

    const uint8_t * p_buffer = buffer;
    // Parse address
    p_buffer = to_uint32(p_buffer, &result->address);
    // Byte count
    result->num_bytes = *p_buffer++;
    if (result->num_bytes > UPDATE_DATA_MAX_BYTE_COUNT) {
        return can_errorcode_invalid_byte_count;
    }
    // Reserved byte
    result->reserved = *p_buffer++;
    // Set pointer to data portion
    result->data = p_buffer;
    // Move beyond the data portion
    p_buffer += UPDATE_DATA_MAX_BYTE_COUNT;
    // Last two bytes are the checksum.
    to_uint16(p_buffer, &result->checksum);

    // Checksum
    if (compute_checksum(buffer, p_buffer) != result->checksum) {
        return can_errorcode_bad_checksum;
    }

    return can_errorcode_ok;
}


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
    UpdateComplete * result) {

    if (!result) {
        return can_errorcode_invalid_input;
    }

    // Message size and null check
    if (!buffer || size != UPDATE_COMPLETE_MESSAGE_SIZE) {
        return can_errorcode_invalid_size;
    }

    to_uint32(buffer, &result->num_messages);

    return can_errorcode_ok;
}
