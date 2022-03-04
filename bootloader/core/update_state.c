#include "bootloader/core/update_state.h"

static UpdateState update_state = {
    .num_messages_received=0,
    .error_detection=0,
};


UpdateState * get_update_state() {
    return &update_state;
}

void reset_update_state(UpdateState * state) {
    if (state) {
        state->num_messages_received = 0;
        state->error_detection = 0;
    }
}