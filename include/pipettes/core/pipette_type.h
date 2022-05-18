#pragma once

typedef enum Type {
    SINGLE_CHANNEL,
    EIGHT_CHANNEL,
    NINETY_SIX_CHANNEL,
    THREE_EIGHTY_FOUR_CHANNEL
} PipetteType;

#ifdef __cplusplus


/**
 * Get the current pipette type
 *
 * @return a pipette type
 */
consteval auto get_pipette_type() -> PipetteType { return PIPETTE_TYPE_DEFINE; }


#else

// this happens if included from a .c file
inline PipetteType get_pipette_type() { return PIPETTE_TYPE_DEFINE; }
#endif  // __cplusplus
