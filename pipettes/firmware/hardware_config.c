#include <stdlib.h>

#include "hardware_config.h"
#include "platform_specific_hal_conf.h"
#include "stm32l5xx_hal_gpio.h"

static PipetteHardwarePin get_gpio_ht(PipetteHardwareDevice device) {
    PipetteHardwarePin pinout;

    switch (device) {
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
            return pinout;
    }
    abort();

}

static uint16_t get_spi_pins_ht(GPIO_TypeDef* for_handle) {
    /*
     * SPI Bus 2
     * clk -> PB13
     * cipo -> PB14
     * copi -> PB15
     *
     * Chip Select
     * plunger -> PC6
     * left pickup -> PC9
     * right pickup -> PC10
     */
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOB: return GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        // PC6, PC8, PC9
        case (uint32_t)GPIOC: return GPIO_PIN_6 | GPIO_PIN_9 | GPIO_PIN_10;
    }
    abort();
}

static PipetteHardwarePin get_gpio_lt(PipetteHardwareDevice device) {
    PipetteHardwarePin pinout;
    switch(device) {
        case pipette_hardware_device_LED_drive:
            pinout.port = GPIOA;
            pinout.pin = GPIO_PIN_8;
            return pinout;
        case pipette_hardware_device_sync_in:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_5;
            return pinout;
        case pipette_hardware_device_sync_out:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_4;
            return pinout;
    }
    abort();
}


static uint16_t get_spi_pins_lt(GPIO_TypeDef* for_handle) {
    /*
     * SPI Bus 2
     * clk -> PB13
     * cipo -> PB14
     * copi -> PB15
     *
     * Chip Select
     * PC6
     */
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOB: return GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        case (uint32_t)GPIOC: return GPIO_PIN_6;
    }
    abort();
}

static uint16_t get_motor_driver_pins_ht(GPIO_TypeDef* for_handle) {
    /*
     * Dir Pins
     * plunger -> PA7
     * left pickup -> PC7
     * right pickup -> PC13
     *
     * Step Pins
     * plunger -> PB10
     * left pickup -> PC8
     * right pickup -> PB8
     *
     * VREF (TMC2130)
     * PA5
     *
     * Motor Enable
     * PD2
     */
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOA: return GPIO_PIN_5 | GPIO_PIN_7;
        case (uint32_t)GPIOB: return GPIO_PIN_8 | GPIO_PIN_10;
        case (uint32_t)GPIOC: return GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_13;
        case (uint32_t)GPIOD: return GPIO_PIN_2;
    }
    abort();
}

static uint16_t get_motor_driver_pins_lt(GPIO_TypeDef* for_handle) {
    /*
     * Dir Pin -> PC3
     *
     * Step Pin -> PC7
     * Enable Pin -> PC8
     *
     * VREF (TMC2130)
     * PA5
     */
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOA: return GPIO_PIN_5;
        case (uint32_t)GPIOC: return GPIO_PIN_3 | GPIO_PIN_7 | GPIO_PIN_8;
    }
    abort();
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

uint16_t pipette_hardware_motor_driver_pins(PipetteType pipette_type, GPIO_TypeDef* for_handle) {
    switch (pipette_type) {
        case NINETY_SIX_CHANNEL:
        case THREE_EIGHTY_FOUR_CHANNEL:
            return get_motor_driver_pins_ht(for_handle);
        case SINGLE_CHANNEL:
        case EIGHT_CHANNEL:
        default:
            return get_motor_driver_pins_lt(for_handle);
    }
}