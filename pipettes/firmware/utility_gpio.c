#include "common/firmware/utility_gpio.h"
#include "platform_specific_hal_conf.h"
#include "pipettes/core/pipette_info.hpp"

/**
 * @brief Limit Switch GPIO Initialization Function
 * @param None
 * @retval None
 */
void limit_switch_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /*Configure GPIO pin*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = gpio_pins.limit_switch.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(gpio_pins.limit_switch.port, &GPIO_InitStruct);
}

/**
 * @brief LED GPIO Initialization Function
 * @param None
 * @retval None
 */
void LED_drive_gpio_init() {
    /* GPIO Ports Clock Enable */
    if (gpio_pins.led_drive.port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (gpio_pins.led_drive.port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }

    /*Configure GPIO pin*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = gpio_pins.led_drive.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(gpio_pins.led_drive.port, &GPIO_InitStruct);
}

void sync_drive_gpio_init() {
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = gpio_pins.sync_in.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(gpio_pins.sync_in.port, &GPIO_InitStruct);


void utility_gpio_init(void) {
    limit_switch_gpio_init();
    LED_drive_gpio_init();
    sync_drive_gpio_init();
}