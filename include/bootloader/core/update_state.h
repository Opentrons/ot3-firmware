#pragma once

#include <stdint.h>

#ifndef __cplusplus
#include <stdatomic.h>
#else
// _Atomic keyword is not supported in CPP and we don't need it.
#define _Atomic
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


typedef enum {
    erase_state_idle,
    erase_state_running,
    erase_state_error,
    erase_state_done,
} EraseState;


typedef struct {
    /** Number of data messages received. */
    uint32_t num_messages_received;
    /** Running error detection value of update date. */
    uint32_t error_detection;
    /** The current flash erasing state. Marked `_Atomic` because it's
     * modified in interrupt and main process. */
    _Atomic EraseState erase_state;

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
