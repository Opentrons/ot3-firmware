#include <stdint.h>
#include "bootloader/core/util.h"


/**
 * Extract a uint32 from byte buffer
 * @param buffer pointer to buffer
 * @param result pointer to result
 * @return pointer to position after the uint32
 */
const uint8_t * to_uint32(const uint8_t * buffer, uint32_t * result) {
    if (!buffer || !result) {
        return buffer;
    }
    *result = 0;
    *result |= (*buffer++ << 24);
    *result |= (*buffer++ << 16);
    *result |= (*buffer++ << 8);
    *result |= *buffer++;
    return buffer;
}

/**
 * Extract a uint16 from byte buffer
 * @param buffer pointer to buffer
 * @param result pointer to result
 * @return pointer to position after the uint16
 */
const uint8_t * to_uint16(const uint8_t * buffer, uint16_t * result) {
    if (!buffer || !result) {
        return buffer;
    }
    *result = 0;
    *result |= (*buffer++ << 8);
    *result |= *buffer++;
    return buffer;
}

/**
 * Write uint16 into buffer
 * @param buffer pointer to buffer
 * @param value value
 * @return pointer to position after the uint16
 */
uint8_t * write_uint16(uint8_t * buffer, uint16_t value) {
    if (!buffer) {
        return buffer;
    }
    *(buffer++) = (value >> 8) & 0xFF;
    *(buffer++) = value & 0xFF;
    return buffer;
}

/**
 * Write uint32 into buffer
 * @param buffer pointer to buffer
 * @param value value
 * @return pointer to position after the uint32
 */
uint8_t * write_uint32(uint8_t * buffer, uint32_t value) {
    if (!buffer) {
        return buffer;
    }
    *(buffer++) = (value >> 24) & 0xFF;
    *(buffer++) = (value >> 16) & 0xFF;
    *(buffer++) = (value >> 8) & 0xFF;
    *(buffer++) = value & 0xFF;
    return buffer;
}

/**
 * Compute checksum of bytes.
 */
uint16_t compute_checksum(const uint8_t * begin, const uint8_t * end) {
    int32_t computed_checksum = 0;
    for (; begin < end; begin++) {
        computed_checksum += *begin;
    }
    return (~computed_checksum + 1) & 0xFFFF;
}
