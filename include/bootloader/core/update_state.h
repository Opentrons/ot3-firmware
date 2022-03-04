#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


typedef struct {
    /** Number of data messages received. */
    uint32_t num_messages_received;
    /** Running error detection value of update date. */
    uint32_t error_detection;
} UpdateState;


/**
 * Get the firmware update state.
 * @return Pointer to the singleton
 */
UpdateState * get_update_state();


/**
 * Reset the state to default;
 * @param state pointer to state
 */
void reset_update_state(UpdateState * state);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
