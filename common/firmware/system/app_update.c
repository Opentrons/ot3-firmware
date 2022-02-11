#include <stdint.h>
#include <stdbool.h>
#include "platform_specific_hal_conf.h"
#include "platform_specific_hal.h"
#include "common/core/app_update.h"


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

    // Disable irqs.
    __disable_irq();

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
    return firmware_update_flags & UPDATE_FLAG_REQUESTED;
}


/** Clear the update flags. */
void app_update_clear_flags() {
    firmware_update_flags = UPDATE_FLAG_NONE;
}
