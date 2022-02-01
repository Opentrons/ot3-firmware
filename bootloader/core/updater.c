#include "bootloader/core/updater.h"



FwUpdateReturn fw_update_initialize(void) {
    return fw_update_error;
}


FwUpdateReturn fw_update_data(uint32_t address, const uint8_t* data, uint8_t length) {
    return fw_update_error;
}


FwUpdateReturn fw_update_complete(uint32_t num_messages, uint32_t crc32) {
    return fw_update_error;
}