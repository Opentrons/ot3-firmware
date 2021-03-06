#include "platform_specific_hal_conf.h"
#include "platform_specific_hal.h"
#include "common/firmware/iwdg.h"
#include "bootloader/core/updater.h"
#include "bootloader/core/util.h"
#include "bootloader/firmware/constants.h"
#include "bootloader/firmware/crc32.h"


/**
 * Callback to buffer iterator that writes a value to flash.
 * @param address The address to write to
 * @param data The 64bit value
 * @return true on success
 */
static bool fw_write_to_flash(uint32_t address, uint64_t data);


/**
 * Wait for erase to complete.
 */
static void fw_update_wait_erase(const UpdateState* state);


FwUpdateReturn fw_update_initialize(UpdateState* state) {
    if (!state) {
        return fw_update_error;
    }
    reset_update_state(state);

    crc32_reset_accumulator();

    return fw_update_ok;
}


FwUpdateReturn fw_update_data(UpdateState* state, uint32_t address, const uint8_t* data, uint8_t length) {
    if (!state) {
        return fw_update_error;
    }

    // Update CRC value
    state->error_detection = crc32_accumulate(data, length);

    // TODO (amit, 2022-02-01): Validate the address. Don't overwrite something horrible.

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return fw_update_error;
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
        return fw_update_invalid_size;
    }
    if (error_detection != state->error_detection) {
        return fw_update_invalid_data;
    }
    return fw_update_ok;
}


FwUpdateReturn fw_update_erase_application(UpdateState* state) {
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return fw_update_error;
    }

#ifndef FLASH_BANK_2
    // Single bank flash mode. Erase all the pages above the bootloader area.
    FLASH_EraseInitTypeDef erase_struct =  {
        .TypeErase=FLASH_TYPEERASE_PAGES,
        .Banks=FLASH_BANK_1,
        .Page=APP_START_PAGE,
        .NbPages=APP_NUM_PAGES
    };

    state->erase_state = erase_state_running;

    if (HAL_FLASHEx_Erase_IT(&erase_struct) != HAL_OK) {
        return fw_update_error;
    }
#else
    // Dual bank flash mode.
    // Erase all the pages above the bootloader area in the first bank.
    FLASH_EraseInitTypeDef erase_struct =  {
        .TypeErase=FLASH_TYPEERASE_PAGES,
        .Banks=FLASH_BANK_1,
        .Page=APP_START_PAGE,
        .NbPages=FLASH_PAGE_NB_PER_BANK - APP_START_PAGE
    };

    state->erase_state = erase_state_running;

    if (HAL_FLASHEx_Erase_IT(&erase_struct) != HAL_OK) {
        return fw_update_error;
    }

    // Wait for first erase to complete.
    fw_update_wait_erase(state);
    if (state->erase_state == erase_state_error) {
        return fw_update_error;
    }

    // Erase the second bank.
    erase_struct.Banks=FLASH_BANK_2;
    erase_struct.Page=0;
    erase_struct.NbPages=FLASH_PAGE_NB_PER_BANK;

    if (HAL_FLASHEx_Erase_IT(&erase_struct) != HAL_OK) {
        return fw_update_error;
    }
#endif
    fw_update_wait_erase(state);
    if (state->erase_state == erase_state_error) {
        return fw_update_error;
    }

    return fw_update_ok;
}


void fw_update_wait_erase(const UpdateState * state) {
    while (state->erase_state == erase_state_running) {
        HAL_Delay(100);
    }
}

/**
 * Callback from HAL_FLASH_IRQHandler indicating that an operation is
 * complete.
 * @param ReturnValue This is overloaded to mean a whole heck of a lot.
 *  for page erase it is the page (or 0xFFFFFFFF indicating complete)
 *  for mass erase this is the bank. (not in use by us)
 *  for program it is the address. (not in use by us)
 */
void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue) {
    (void)ReturnValue;
    // If we're erasing and getting the magic ReturnValue then we're done.
    if (get_update_state()->erase_state == erase_state_running && ReturnValue == 0xFFFFFFFFU) {
        get_update_state()->erase_state = erase_state_done;
    }
    // Refresh the watch dog
    iwdg_refresh();
}

/**
 * Callback from HAL_FLASH_IRQHandler indicating that an error has happened.
 * @param ReturnValue his is overloaded to mean:
 *   page for page erase
 *   bank for mass erase (not in use by us)
 *   address for program (not in use by us)
 */
void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue) {
    (void)ReturnValue;
    // If we're erasing then we've got an error on our hands.
    if (get_update_state()->erase_state == erase_state_running) {
        get_update_state()->erase_state = erase_state_error;
    }
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
