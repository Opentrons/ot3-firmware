#include "gpio.h"
#include "platform_specific_hal.h"
#include <stdbool.h>

__attribute__((section( ".ccmram" )))
void gpio_set(void* port, uint16_t pin, uint8_t active_setting) {
    if (!port) {
        return;
    }
    HAL_GPIO_WritePin(port, pin, active_setting);
}

__attribute__((section( ".ccmram" )))
static uint8_t invert_gpio_value(uint8_t setting) {
    switch (setting) {
        case GPIO_PIN_SET:
            return GPIO_PIN_RESET;
        case GPIO_PIN_RESET:
            return GPIO_PIN_SET;
        default:
            return GPIO_PIN_SET;
    }
}

__attribute__((section( ".ccmram" )))
void gpio_reset(void* port, uint16_t pin,
                uint8_t active_setting) {
    if (!port) {
        return;
    }
    HAL_GPIO_WritePin(port, pin, invert_gpio_value(active_setting));
}

__attribute__((section( ".ccmram" )))
bool gpio_is_set(void* port, uint16_t pin, uint8_t active_setting) {
    return HAL_GPIO_ReadPin(port, pin) == active_setting;
}
