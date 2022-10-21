#include "hardware_config.h"
#include "platform_specific_hal_conf.h"
#include "stm32g4xx_hal_gpio.h"

static PipetteHardwarePin get_gpio_ht(PipetteHardwareDevice device) {
    PipetteHardwarePin pinout;

    switch (device) {
        case pipette_hardware_device_LED_drive:
            pinout.port = GPIOC;
            pinout.pin = GPIO_PIN_14;
            return pinout;
        case pipette_hardware_device_sync_in:
            pinout.port = GPIOD;
            pinout.pin = GPIO_PIN_2;
            return pinout;
        case pipette_hardware_device_sync_out:
            pinout.port = GPIOA;
            pinout.pin = GPIO_PIN_9;
            return pinout;
        case pipette_hardware_device_data_ready_front:
            pinout.port = GPIOC;
            pinout.pin = GPIO_PIN_11;
            return pinout;
        case pipette_hardware_device_data_ready_rear:
            pinout.port = GPIOC;
            pinout.pin = GPIO_PIN_6;
            return pinout;
        default:
            pinout.port = 0;
            pinout.pin = 0;
            return pinout;
    }
}

IRQn_Type get_interrupt_line(const PipetteType pipette_type) {
    switch (pipette_type) {
        case NINETY_SIX_CHANNEL:
        case THREE_EIGHTY_FOUR_CHANNEL:
            // External interrupt lines 15:10
            // TODO(lc: 10-20-2022) This will only
            // return the front sensor board data ready line
            // for now. We need to support and set-up both
            // for the 96-chan and 8-chan
            // PC6 -> GPIO_EXTI6 (EXTI9_5_IRQn)
            // PC11 -> GPIO_EXTI11 (EXTI15_10_IRQn)
            return EXTI15_10_IRQn;
        case SINGLE_CHANNEL:
        case EIGHT_CHANNEL:
        default:
            // External interrupt line 3
            return EXTI3_IRQn;
    }
}


static uint16_t get_spi_pins_ht(GPIO_TypeDef* for_handle) {
    /*
     * SPI Bus 2
     * clk -> PB13
     * cipo -> PB14
     * copi -> PB15
     *
     * Chip Select
     * plunger -> PC5
     * left pickup -> PB11
     * right pickup -> PC15
     *
     * Any CS pins sharing the same port as the spi bus need
     * to be initialized separately since the configurations are different.
     *
     */
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOB: return GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        case (uint32_t)GPIOC: return GPIO_PIN_5 | GPIO_PIN_15;
        default: return 0;
    }
}

static PipetteHardwarePin get_gpio_lt(PipetteHardwareDevice device) {
    PipetteHardwarePin pinout;
    switch(device) {
        case pipette_hardware_device_LED_drive:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_11;
            return pinout;
        case pipette_hardware_device_sync_in:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_7;
            return pinout;
        case pipette_hardware_device_sync_out:
            pinout.port = GPIOB;
            pinout.pin = GPIO_PIN_6;
            return pinout;
        case pipette_hardware_device_data_ready_front:
            pinout.port = GPIOC;
            pinout.pin = GPIO_PIN_3;
            return pinout;
        default:
            pinout.port = 0;
            pinout.pin = 0;
            return pinout;
    }
}


static uint16_t get_spi_pins_lt(GPIO_TypeDef* for_handle) {
    /*
     * SPI Bus 2
     * clk -> PB13
     * cipo -> PB14
     * copi -> PB15
     *
     * Chip Select
     * PB12
     *
     * Any CS pins sharing the same port as the spi bus need
     * to be initialized separately since the configurations are different.
     */
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOB: return GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        default: return 0;
    }
}

static uint16_t get_motor_driver_pins_ht(GPIO_TypeDef* for_handle) {
    /*
     * Dir Pins
     * plunger -> PA6
     * left pickup -> PB0
     * right pickup -> PB7
     *
     * Step Pins
     * plunger -> PA7
     * left pickup -> PB1
     * right pickup -> PC2
     *
     *
     * Motor Enable
     * pickup enable -> PA10
     * plunger enable -> PB4
     */
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOA: return GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
        case (uint32_t)GPIOB: return GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_4;
        case (uint32_t)GPIOC: return GPIO_PIN_2;
        default: return 0;
    }
}

static uint16_t get_motor_driver_pins_lt(GPIO_TypeDef* for_handle) {
    /*
     * Dir Pin -> PC6
     *
     * Step Pin -> PC7
     * Enable Pin -> PA10
     *
     * VREF (TMC2130)
     * PA5
     */
    switch((uint32_t)for_handle) {
        case (uint32_t)GPIOA: return GPIO_PIN_10;
        case (uint32_t)GPIOC: return GPIO_PIN_6 | GPIO_PIN_7;
        default: return 0;
    }
}


PipetteHardwarePin pipette_hardware_get_gpio(const PipetteType pipette_type, PipetteHardwareDevice device) {
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


uint16_t pipette_hardware_spi_pins(const PipetteType pipette_type, GPIO_TypeDef* for_handle) {
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

uint16_t pipette_hardware_motor_driver_pins(const PipetteType pipette_type, GPIO_TypeDef* for_handle) {
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
