#include "platform_specific_hal_conf.h"
#include "platform_specific_hal.h"
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


FwUpdateReturn fw_update_initialize(UpdateState* state) {
    if (!state) {
        return fw_update_error;
    }
    reset_update_state(state);

    // TODO (amit, 2022-02-01): Erase app flash space?
    return fw_update_ok;
}


FwUpdateReturn fw_update_data(UpdateState* state, uint32_t address, const uint8_t* data, uint8_t length) {
    if (!state) {
        return fw_update_error;
    }
    // TODO (amit, 2022-02-01): Update error detection with crc32 or checksum of data.

    // TODO (amit, 2022-02-01): Validate the address. Don't overwrite something horrible.

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return fw_update_error;
    }

    if (!state->erased) {
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
        state->erased = true;
    }

    FwUpdateReturn ret = fw_update_ok;

    if (!dword_address_iter(address, data, length, fw_write_to_flash)) {
        ret = fw_update_error;
    }

    if (HAL_FLASH_Lock() != HAL_OK) {
        ret = fw_update_error;
    }

    state->num_messages_received++;
    return ret;
}


FwUpdateReturn fw_update_complete(UpdateState* state, uint32_t num_messages, uint32_t error_detection) {
    if (!state) {
        return fw_update_error;
    }

    if (num_messages != state->num_messages_received) {
        // TODO (amit, 2022-02-01): Erase app flash space?
        return fw_update_invalid_size;
    }
    if (error_detection != state->error_detection) {
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


#define SYSMEM_START APP_FLASH_ADDRESS
#define SYSMEM_BOOT (SYSMEM_START + 4)

const uint32_t *const sysmem_boot_loc = (uint32_t*)SYSMEM_BOOT;

void fw_update_start_application() {
    // Disable irqs.
    __disable_irq();

    // systick should be off at boot
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    /* Clear Interrupt Enable Register & Interrupt Pending Register */
    for (int i=0;i<8;i++)
    {
        NVIC->ICER[i]=0xFFFFFFFF;
        NVIC->ICPR[i]=0xFFFFFFFF;
    }

    // Initialize user bootloader's Stack Pointer
    __set_MSP(*(__IO uint32_t *) APP_FLASH_ADDRESS);

    /**
     * Unlike in the application, we're not running freertos so we're currently
     * using the MSP, making c function call preambles unsafe, so we have to use
     * asm to jump to the application.
     */
    asm volatile (
        "bx %0"
        : // no outputs
        : "r" (*sysmem_boot_loc)
        : "memory"  );
}