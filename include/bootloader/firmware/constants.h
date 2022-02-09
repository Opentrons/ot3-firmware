#pragma once

// Application offset. The number of bytes allocated to the bootloader.
#define APP_OFFSET  0x8000

// Application flash address
#define APP_FLASH_ADDRESS  (0x08000000 | APP_OFFSET)

// First flash page of application
#define APP_START_PAGE (APP_OFFSET >> 11)

// Number of flash pages allocated to application
#define APP_NUM_PAGES  (FLASH_PAGE_NB - APP_START_PAGE)
