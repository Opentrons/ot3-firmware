#pragma once

/**
 * Contents of the firmware update data message.
 */
struct UpdateData {
    uint32_t address;
    uint8_t num_bytes;
    uint8_t reserved;
    uint8_t data[56];
    uint16_t checksum;
};

/**
 * Contents of the firmware update complete message.
 */
struct UpdateComplete {
    uint32_t num_messages;
};


/**
 * Populate UpdateData fields from buffer. 
 * @param buffer Pointer to a buffer
 * @param size The size of the buffer
 * @param result Pointer to UpdateData struct to populate
 * @return result code
 */
uint16_t parse_update_data(
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
uint16_t parse_update_complete(
    const uint8_t * buffer,
    uint32_t size,
    UpdateData * result);