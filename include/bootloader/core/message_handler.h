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
    handle_message_error
}  HandleMessageReturn;

/**
 * Handle a message
 * @param request The request
 * @param response Where to put the response.
 * @return Return code
 */
HandleMessageReturn handle_message(const Message * request, Message * response);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
