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



/**
 * Iterate a byte buffer creating double word values and addresses.
 *
 * Writes to G4 and L5 flash happen in 64 bit data increments.
 * @param address The starting flash address
 * @param buffer A byte buffer
 * @param length Length in bytes of the buffer.
 * @param callback A callback that will be called for each 64 bit address
 */
void dword_address_iter(uint32_t address, const uint8_t * buffer, uint8_t length, address_iter_callback callback) {
    if (!buffer || length <= 0) {
        return;
    }
    uint64_t double_word = 0;
    for (int i = 0; i < length; i++) {
        if ((i != 0 && 0 == (i % (sizeof(uint64_t))))) {
            // We have a complete double word. Call the callback.
            callback(address, double_word);
            // Clear word.
            double_word = 0;
            // Increment address
            address += sizeof(uint64_t);
        }
        // The number of bytes to shift.
        int shift_bytes = sizeof(uint64_t) - ((i % sizeof(uint64_t)) + 1);
        int shift_amount = shift_bytes * 8;
        // Or the shifted byte into the double word
        double_word |= ((uint64_t)buffer[i] << shift_amount);
    }
    // Send the last address
    callback(address, double_word);
}

