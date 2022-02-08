#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Extract a uint32 from byte buffer
 * @param buffer pointer to buffer
 * @param result pointer to result
 * @return pointer to position after the uint32
 */
const uint8_t * to_uint32(const uint8_t * buffer, uint32_t * result);

/**
 * Extract a uint16 from byte buffer
 * @param buffer pointer to buffer
 * @param result pointer to result
 * @return pointer to position after the uint16
 */
const uint8_t * to_uint16(const uint8_t * buffer, uint16_t * result);

/**
 * Write uint16 into buffer
 * @param buffer pointer to buffer
 * @param value value
 * @return pointer to position after the uint16
 */
uint8_t * write_uint16(uint8_t * buffer, uint16_t value);

/**
 * Write uint32 into buffer
 * @param buffer pointer to buffer
 * @param value value
 * @return pointer to position after the uint32
 */
uint8_t * write_uint32(uint8_t * buffer, uint32_t value);

/**
 * Compute checksum of bytes.
 * @param begin pointer to beginning of buffer
 * @param end pointer to end of buffer
 * @return the checksum
 */
uint16_t compute_checksum(const uint8_t * begin, const uint8_t * end);


/**
 * Callback given to dword_address_iter.
 */
typedef bool (*address_iter_callback) (uint32_t address, uint64_t data);

/**
 * Iterate a byte buffer creating double word values and addresses.
 *
 * Writes to G4 and L5 flash happen in 64 bit data increments.
 * @param address The starting flash address
 * @param buffer A byte buffer
 * @param length Length in bytes of the buffer.
 * @param callback A callback that will be called for each 64 bit address
 * @return True on success
 */
bool dword_address_iter(uint32_t address, const uint8_t * buffer, uint8_t length, address_iter_callback callback);


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
