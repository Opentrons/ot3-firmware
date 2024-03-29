#include "bootloader/core/messages.h"
#include "bootloader/core/util.h"

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
    uint32_t * result) {
    if (!result) {
        return can_errorcode_invalid_input;
    }

    // Message size and null check
    if (!buffer || size != sizeof(uint32_t)) {
        return can_errorcode_invalid_size;
    }
    // parse message_index
    to_uint32(buffer, result);

    return can_errorcode_ok;
}

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
        // if its there, populate with message index
        if (buffer && size > sizeof(result->message_index)) {
            buffer = to_uint32(buffer, &result->message_index);
        }
        return can_errorcode_invalid_size;
    }

    const uint8_t * p_buffer = buffer;
    // parse message_index
    p_buffer = to_uint32(p_buffer, &result->message_index);
    // Parse address
    p_buffer = to_uint32(p_buffer, &result->address);
    // the write address needs to be doubleword aligned
    if (result->address % 8 != 0) {
        return can_errorcode_invalid_input;
    }
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
        // if its there, populate with message index
        if (buffer && size > sizeof(result->message_index)) {
            buffer = to_uint32(buffer, &result->message_index);
        }
        return can_errorcode_invalid_size;
    }
    buffer = to_uint32(buffer, &result->message_index);
    buffer = to_uint32(buffer, &result->num_messages);
    to_uint32(buffer, &result->crc32);

    return can_errorcode_ok;
}
