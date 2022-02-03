#pragma once


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

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
