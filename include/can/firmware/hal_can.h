#pragma once

#include "can/core/types.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Receive message callback.
 */
typedef void(*can_message_callback)(void* cb_data, uint32_t identifier, uint8_t* data, uint8_t length);

/**
 * HAL can handle type.
 */
typedef void * HAL_CAN_HANDLE;



/**
 * Start CAN.
 */
void can_start(uint8_t clock_divider, uint8_t segment_1_tqs, uint8_t segment_2_tqs, uint8_t max_sync_jump_width);


/**
 * Get CAN device handle.
 * @return a handle to a device.
 */
HAL_CAN_HANDLE can_get_device_handle();


/**
 * Register a callback function to process new messages.
 *
 * This is called from an ISR
 *
 * @param cb_data a value that will be passed to the callback function.
 * @param callback a callback function.
 */
void can_register_message_callback(void * cb_data, can_message_callback callback);


/**
 * Send a can message.
 *
 * @param handle handle to can device
 * @param arbitration_id the arbitration id
 * @param buffer pointer to start of buffer
 * @param buffer_length length of buffer as defined by HAL
 */
void can_send_message(HAL_CAN_HANDLE handle, uint32_t arbitration_id, uint8_t* buffer,
                      CanFDMessageLength buffer_length);



/**
 * Add a filter for incoming messages.
 *
 * @param handle handle to can device
 * @param index index of the filter
 * @param type the filter type (range, mask, exact match).
 * @param config what the filter should do
 * @param val1 filter value
 * @param val2 filter value
 */
void can_add_filter(HAL_CAN_HANDLE handle, uint32_t index, CanFilterType type, CanFilterConfig config,
                    uint32_t val1, uint32_t val2);



#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

