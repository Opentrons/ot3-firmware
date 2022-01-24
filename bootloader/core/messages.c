#include "bootloader/core/messages.h"

static const uint8_t * to_uint32(const uint8_t * buffer, uint32_t * result) {
    *result = 0;
    *result |= (*buffer++ << 24);
    *result |= (*buffer++ << 16);
    *result |= (*buffer++ << 8);
    *result |= *buffer++;
    return buffer;
}

static const uint8_t * to_uint16(const uint8_t * buffer, uint16_t * result) {
    *result = 0;
    *result |= (*buffer++ << 8);
    *result |= *buffer++;
    return buffer;
}

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
    struct UpdateData * result) {

    // Message size must be the max fdcan length
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
    int32_t computed_checksum = 0;
    for (const uint8_t * p = buffer; p < p_buffer; p++) {
        computed_checksum += *p;
    }
    computed_checksum = (~computed_checksum + 1) & 0xFFFF;
    if (computed_checksum != result->checksum) {
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
enum ErrorCode parse_update_complete(
    const uint8_t * buffer,
    uint32_t size,
    struct UpdateData * result) {
    return can_errorcode_ok;
}