#pragma once

#include <stdint.h>
#include "bootloader/core/ids.h"


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


/**
 * Arbitration ID type.
 */
typedef union {
    CANArbitrationIdParts parts;
    uint32_t id;
} CANArbitrationId;

/**
 * Message definition.
 */
typedef struct {
    CANArbitrationId arbitration_id;
    uint8_t size;
    uint8_t data[64];
} Message;


typedef enum {
    handle_message_ok,
    handle_message_has_response,
    handle_message_error,
    handle_message_not_handled,
}  HandleMessageReturn;

/**
 * Handle a message
 * @param request The request
 * @param response Where to put the response.
 * @return Return code
 */
HandleMessageReturn handle_message(const Message * request, Message * response);

/**
 * The message handler for the system. Useful to call in a system-specific
 * handler if you need to override part of what it returns.
 * @param request The request; check that it is non-null.
 * @param response The response to be filled out; check that it is non-null.
 * */
HandleMessageReturn system_handle_message(const Message * request, Message * response);

/**
 * Systems may implement to customize their message handling.
 *
 * This will be called before the core handle_message; it should return
 * handle_message_not_handled if there was nothing to do.
 *
 * The message pointers will be non-null; they're checked before call.
 * */
HandleMessageReturn system_specific_handle_message(
    const Message* request, Message* response);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
