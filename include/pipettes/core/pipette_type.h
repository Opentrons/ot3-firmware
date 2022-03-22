#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum Type {
    SINGLE_CHANNEL,
    EIGHT_CHANNEL,
    NINETY_SIX_CHANNEL,
    THREE_EIGHTY_FOUR_CHANNEL
} PipetteType;

/**
 * Get the current pipette type
 *
 * @return a pipette type
 */
PipetteType get_pipette_type();


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus