#include <string.h>
#include "bootloader/core/updater.h"


static auto num_messages_received = 0;


FwUpdateReturn fw_update_initialize(void) {
    num_messages_received = 0;
    return fw_update_ok;
}


FwUpdateReturn fw_update_data(uint32_t address, const uint8_t* data, uint8_t length) {
    num_messages_received++;
    return fw_update_ok;
}


FwUpdateReturn fw_update_complete(uint32_t num_messages, uint32_t error_detection) {
    if (num_messages != num_messages_received) {
        return fw_update_invalid_size;
    }
    return fw_update_ok;
}
