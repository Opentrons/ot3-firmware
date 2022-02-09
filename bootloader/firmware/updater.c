#include "platform_specific_hal_conf.h"
#include "bootloader/core/updater.h"
#include "bootloader/core/util.h"
#include "bootloader/firmware/constants.h"


/**
 * Callback to buffer iterator that writes a value to flash.
 * @param address The address to write to
 * @param data The 64bit value
 * @return true on success
 */
static bool fw_write_to_flash(uint32_t address, uint64_t data);


typedef struct {
    /** Number of data messages received. */
    uint32_t num_messages_received;
    /** Running error detection value of update date. */
    uint32_t error_detection;
    bool erased;
} UpdateState;

static UpdateState update_state = {
    .num_messages_received=0,
    .error_detection=0,
    .erased=false
};


FwUpdateReturn fw_update_initialize(void) {
    update_state.num_messages_received=0;
    update_state.error_detection=0;
    update_state.erased=false;

    // TODO (amit, 2022-02-01): Erase app flash space?
    return fw_update_ok;
}


FwUpdateReturn fw_update_data(uint32_t address, const uint8_t* data, uint8_t length) {

    // TODO (amit, 2022-02-01): Update error detection with crc32 or checksum of data.

    // TODO (amit, 2022-02-01): Validate the address. Don't overwrite something horrible.

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return fw_update_error;
    }

    if (!update_state.erased) {
        FLASH_EraseInitTypeDef erase_struct =  {
            .TypeErase=FLASH_TYPEERASE_PAGES,
            .Banks=FLASH_BANK_1,
            .Page=APP_START_PAGE,
            .NbPages=APP_NUM_PAGES
        };
        uint32_t error = 0;
        if (HAL_FLASHEx_Erase(&erase_struct, &error) != HAL_OK) {
            return fw_update_error;
        }
        update_state.erased = true;
    }

    FwUpdateReturn ret = fw_update_ok;

    if (!dword_address_iter(address, data, length, fw_write_to_flash)) {
        ret = fw_update_error;
    }

    if (HAL_FLASH_Lock() != HAL_OK) {
        ret = fw_update_error;
    }

    update_state.num_messages_received++;
    return ret;
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


bool fw_write_to_flash(uint32_t address, uint64_t data) {
    return HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
                             address,
                             data) == HAL_OK;
}
