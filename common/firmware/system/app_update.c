#include <stdint.h>
#include <stdbool.h>
#include "platform_specific_hal_conf.h"
#include "platform_specific_hal.h"
#include "common/core/app_update.h"
#include "bootloader/firmware/constants.h"


/** Fixed location in RAM that stores firmware update flags. */
static update_flag_type __attribute__(( section(".fw_update_flag_section") )) firmware_update_flags;


// The bootloader resides at the beginning of flash.
#define SYSMEM_START 0x08000000
// address 4 in the bootable region is the address of the first instruction that
// should run, aka the data that should be loaded into $pc.
#define SYSMEM_BOOT (SYSMEM_START + 4)

typedef void (*pFunction)(void);
pFunction JumpToBootloader;
uint32_t JumpAddress;

void app_update_start() {
    // Notify bootloader that an update is requested by main application.
    firmware_update_flags = UPDATE_FLAG_REQUESTED;

    // Clear reset flags for bootloader.
    __HAL_RCC_CLEAR_RESET_FLAGS();

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

    // Set the bootloader jump address.
    JumpAddress = *(__IO uint32_t *) (SYSMEM_BOOT);
    JumpToBootloader = (pFunction) JumpAddress;

    // Initialize user bootloader's Stack Pointer
    __set_MSP(*(__IO uint32_t *) SYSMEM_START);

    JumpToBootloader();
}


update_flag_type app_update_flags() {
    return firmware_update_flags;
}


bool is_app_update_requested() {
    return firmware_update_flags == UPDATE_FLAG_REQUESTED;
}


void app_update_clear_flags() {
    firmware_update_flags = UPDATE_FLAG_NONE;
}


bool is_app_in_flash() {
    // Check if start of flash points to RAM.
    if (((*(__IO uint32_t *) APP_FLASH_ADDRESS) & 0x2FFC0000) == 0x20000000) {
        return true;
    }
    return false;
}
