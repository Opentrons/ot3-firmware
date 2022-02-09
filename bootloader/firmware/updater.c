#include "platform_specific_hal_conf.h"
#include "bootloader/core/updater.h"
#include <string.h>


typedef struct {
    /** Number of data messages received. */
    uint32_t num_messages_received;
    /** Running error detection value of update date. */
    uint32_t error_detection;
    int erased;
} UpdateState;

static UpdateState update_state = {.num_messages_received=0, .error_detection=0, .erased=0};


FwUpdateReturn fw_update_initialize(void) {
    memset(&update_state, 0, sizeof(update_state));
    // TODO (amit, 2022-02-01): Erase app flash space?
    return fw_update_ok;
}



FwUpdateReturn fw_update_data(uint32_t address, const uint8_t* data, uint8_t length) {

    if (!update_state.erased) {
        FLASH_EraseInitTypeDef erase_struct =  {
            .TypeErase=FLASH_TYPEERASE_PAGES,
            .Banks=FLASH_BANK_1,
            .Page=0,
            .NbPages=FLASH_PAGE_NB
        };
        uint32_t error;
        if (HAL_FLASHEx_Erase(&erase_struct, &error) != HAL_OK) {
            return fw_update_error;
        }
        update_state.erased = 1;
    }

    // TODO (amit, 2022-02-01): Update error detection with crc32 or checksum of data.
    // TODO (amit, 2022-02-01): Validate the address. Don't overwrite something horrible.

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return fw_update_error;
    }

    uint64_t double_word = 0;

    HAL_StatusTypeDef ret = HAL_OK;
    for(int i = 0; i < length; i++ ) {
        double_word |= (*data[i] << (sizeof(double_word) );



        ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
                                address + i,
                                data32[i]);
        if (ret != HAL_OK) {
            break;
        }
    }

    HAL_FLASH_Lock();

    if (ret != HAL_OK) {
        return fw_update_error;
    }

    update_state.num_messages_received++;
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
