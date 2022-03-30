#include "hardware_config.h"
#include "stm32l5xx_hal_gpio.h"


PipetteHardwarePin pipette_hardware_get_gpio(PipetteHardwareDevice device) {
    PipetteHardwarePin pinout;
    switch(device) {
        case pipette_hardware_device_limit_switch:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_5;
        case pipette_hardware_device_LED_drive:
            pinout.port = GPIOA;
            pinout.pin = GPIO_PIN_8;
        case pipette_hardware_device_sync_in:
            pinout.port = GPIOC;
            pinout.pin = GPIO_PIN_2;
    }
    return pinout;
}


uint16_t pipette_hardware_spi_pins(GPIO_TypeDef* for_handle) {
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOB: return GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        case (uint32_t)GPIOC: return GPIO_PIN_6 | GPIO_PIN_3 | GPIO_PIN_7 | GPIO_PIN_8;
        default: return 0;
    }
}