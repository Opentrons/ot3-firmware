#include <string.h>
#include "bootloader/core/updater.h"


typedef struct {
    uint32_t num_messages_received;
    uint32_t error_detection;
} UpdateState;

static UpdateState update_state;


FwUpdateReturn fw_update_initialize(void) {
    memset(&update_state, 0, sizeof(update_state));
    // TODO (amit, 2022-02-01): Erase app flash space?
    return fw_update_ok;
}


FwUpdateReturn fw_update_data(uint32_t address, const uint8_t* data, uint8_t length) {
    update_state.num_messages_received++;
    // TODO (amit, 2022-02-01): Update error detection with crc32 or checksum of data.
    // update_state.error_detection;
    // TODO (amit, 2022-02-01): Validate the address. Don't overwrite something horrible.
    // TODO (amit, 2022-02-01): Write to flash.
    return fw_update_ok;
}


FwUpdateReturn fw_update_complete(uint32_t num_messages, uint32_t error_detection) {
    if (num_messages != update_state.num_messages_received) {
        // TODO (amit, 2022-02-01): Erase app flash space?
        return fw_update_invalid_size;
    }
    if (error_detection != update_state.error_detection) {
        // TODO (amit, 2022-02-01): Erase app flash space?
        return fw_update_invalid_data;
    }
    // TODO (amit, 2022-02-01): Finalize update, but do not start app. We want
    //  CAN response to go back to host first.
    return fw_update_ok;
}