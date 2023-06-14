#pragma once

#include <stdbool.h>

namespace attached_instruments {

/**
 * @brief Track what instruments are attached, according to the
 * state manager. This includes a
 *
 */
struct AttachedInstruments {
    bool pipette_left = false;
    bool pipette_right = false;
    bool gripper = false;
};

};  // namespace attached_instruments