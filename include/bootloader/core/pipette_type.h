#pragma once

typedef enum Type {
    SINGLE_CHANNEL,
    EIGHT_CHANNEL,
    NINETY_SIX_CHANNEL,
    THREE_EIGHTY_FOUR_CHANNEL
} PipetteType;


#ifdef PIPETTE_TYPE_DEFINE

static inline PipetteType get_pipette_type() { return PIPETTE_TYPE_DEFINE; }

#else

static inline PipetteType get_pipette_type() { return SINGLE_CHANNEL; }
#endif  // pipette type define


