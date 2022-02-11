#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


/** Application update flags stored at fixed location in RAM. */
#define UPDATE_FLAG_NONE         0
#define UPDATE_FLAG_REQUESTED    1

typedef uint32_t update_flag_type;


/**
 * Start an update of the application firmware. Defined here but implemented in common firmware and
 * simulation.
 */
void app_update_start();


/** Get the update flags. */
update_flag_type app_update_flags();


/** Has an app firmware update been requested. */
bool is_app_update_requested();


/** Clear the update flags. */
void app_update_clear_flags();



#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
