#include "bootloader/core/update_state.h"

static UpdateState update_state = {
    .num_messages_received=0,
    .error_detection=0,
    .erased=false
};


UpdateState * get_update_state() {
    return &update_state;
}