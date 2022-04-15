#include "hardware_config.h"
#include "platform_specific_hal_conf.h"
#include "stm32l5xx_hal_gpio.h"

static PipetteHardwarePin get_gpio_ht(PipetteHardwareDevice device) {
    PipetteHardwarePin pinout;

    switch (device) {
        case pipette_hardware_device_limit_switch:
            pinout.port = GPIOC;
            pinout.pin = GPIO_PIN_2;
            return pinout;
        case pipette_hardware_device_LED_drive:
            pinout.port = GPIOC;
            pinout.pin = GPIO_PIN_11;
            return pinout;
        case pipette_hardware_device_sync_in:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_4;
            return pinout;
        case pipette_hardware_device_sync_out:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_5;
        default: return pinout;
    }

}

static uint16_t get_spi_pins_ht(GPIO_TypeDef* for_handle) {
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOB: return GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        case (uint32_t)GPIOC: return GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
        case (uint32_t)GPIOD: return GPIO_PIN_2;
        default: return 0;
    }
}

static PipetteHardwarePin get_gpio_lt(PipetteHardwareDevice device) {
    PipetteHardwarePin pinout;
    switch(device) {
        case pipette_hardware_device_limit_switch:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_5;
            return pinout;
        case pipette_hardware_device_LED_drive:
            pinout.port = GPIOA;
            pinout.pin = GPIO_PIN_8;
            return pinout;
        case pipette_hardware_device_sync_in:
            pinout.port = GPIOC;
            pinout.pin = GPIO_PIN_5;
            return pinout;
        case pipette_hardware_device_sync_out:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_4;
        default: return pinout;
    }
}


static uint16_t get_spi_pins_lt(GPIO_TypeDef* for_handle) {
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOB: return GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        case (uint32_t)GPIOC: return GPIO_PIN_6 | GPIO_PIN_3 | GPIO_PIN_7 | GPIO_PIN_8;
        default: return 0;
    }
}


PipetteHardwarePin pipette_hardware_get_gpio(PipetteType pipette_type, PipetteHardwareDevice device) {
    switch (pipette_type) {
        case NINETY_SIX_CHANNEL:
        case THREE_EIGHTY_FOUR_CHANNEL:
            return get_gpio_ht(device);
        case SINGLE_CHANNEL:
        case EIGHT_CHANNEL:
        default:
            return get_gpio_lt(device);
    }
}


uint16_t pipette_hardware_spi_pins(PipetteType pipette_type, GPIO_TypeDef* for_handle) {
    switch (pipette_type) {
        case NINETY_SIX_CHANNEL:
        case THREE_EIGHTY_FOUR_CHANNEL:
            return get_spi_pins_ht(for_handle);
        case SINGLE_CHANNEL:
        case EIGHT_CHANNEL:
        default:
            return get_spi_pins_lt(for_handle);
    }
}
