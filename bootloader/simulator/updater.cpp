#include "bootloader/core/updater.h"

#include "bootloader/core/update_state.h"

FwUpdateReturn fw_update_initialize(UpdateState* state) {
    reset_update_state(state);
    return fw_update_ok;
}

FwUpdateReturn fw_update_data(UpdateState* state, uint32_t, const uint8_t*,
                              uint8_t) {
    state->num_messages_received++;
    return fw_update_ok;
}

FwUpdateReturn fw_update_complete(UpdateState* state, uint32_t num_messages,
                                  uint32_t) {
    if (num_messages != state->num_messages_received) {
        return fw_update_invalid_size;
    }
    return fw_update_ok;
}

void fw_update_start_application() {}

FwUpdateReturn fw_update_erase_application(UpdateState*) {
    return fw_update_ok;
}
