#include "bootloader/core/updater.h"

#include "bootloader/core/update_state.h"

FwUpdateReturn fw_update_initialize(UpdateState* state) {
    reset_update_state(state);
    return fw_update_ok;
}

FwUpdateReturn fw_update_data(UpdateState* state, uint32_t address,
                              const uint8_t* data, uint8_t length) {
    state->num_messages_received++;
    return fw_update_ok;
}

FwUpdateReturn fw_update_complete(UpdateState* state, uint32_t num_messages,
                                  uint32_t error_detection) {
    if (num_messages != state->num_messages_received) {
        return fw_update_invalid_size;
    }
    return fw_update_ok;
}
