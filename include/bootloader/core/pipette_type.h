#pragma once

typedef enum Type {
    SINGLE_CHANNEL,
    EIGHT_CHANNEL,
    NINETY_SIX_CHANNEL,
    THREE_EIGHTY_FOUR_CHANNEL
} PipetteType;

static inline PipetteType get_pipette_type() { return PIPETTE_TYPE_DEFINE; }
