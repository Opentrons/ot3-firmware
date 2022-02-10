#include <stdint.h>
#include <stdbool.h>
#include "platform_specific_hal_conf.h"
#include "platform_specific_hal.h"
#include "common/core/app_update.h"


/** Fixed location in RAM that stores firmware update flags. */
static update_flag_type __attribute__(( section(".fw_update_flag_section") )) firmware_update_flags;


// (Code taken from opentrons-modules)
// This is the start of the sys memory region for the STM32G491 and STM32L562
// from the reference manual and STM application note AN2606
#define SYSMEM_START 0x1fff0000
#define SYSMEM_BOOT (SYSMEM_START + 4)

// address 4 in the bootable region is the address of the first instruction that
// should run, aka the data that should be loaded into $pc.
const uint32_t *const sysmem_boot_loc = (uint32_t*)SYSMEM_BOOT;


void app_update_start() {
    // Notify bootloader that an update is requested by main application.
    firmware_update_flags = UPDATE_FLAG_REQUESTED;

    // (Code taken from opentrons-modules)
    // We have to uninitialize as many of the peripherals as possible, because the bootloader
    // expects to start as the system comes up

    // The HAL has ways to turn off all the core clocking and the clock security system
    HAL_RCC_DisableLSECSS();
    HAL_RCC_DeInit();

    // systick should be off at boot
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // We have to make sure that the processor is mapping the system memory region to address 0,
    // which the bootloader expects
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
    // and now we're ready to set the system up to start executing system flash.
    // arm cortex initialization means that

    // address 0 in the bootable region is the address where the processor should start its stack
    // which we have to do as late as possible because as soon as we do this the c and c++ runtime
    // environment is no longer valid
    __set_MSP(*((uint32_t*)SYSMEM_START));

    // finally, jump to the bootloader. we do this in inline asm because we need
    // this to be a naked call (no caller-side prep like stacking return addresses)
    // and to have a naked function you need to define it as a function, not a
    // function pointer, and we don't statically know the address here since it is
    // whatever's contained in that second word of the bsystem memory region.
    asm volatile (
        "bx %0"
        : // no outputs
        : "r" (*sysmem_boot_loc)
        : "memory"  );
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
